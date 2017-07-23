#include <stdio.h>

#include <libavformat/avio.h>
#include <libavformat/avformat.h>
#include <libavutil/intreadwrite.h>

#include <dvdread/nav_print.h>

#include "common.h"

int find_next_start_code(AVIOContext *pb, int *size_ptr,
                         int32_t *header_state)
{
    unsigned int state, v;
    int val, n;

    state = *header_state;
    n     = *size_ptr;
    while (n > 0) {
        if (pb->eof_reached)
            break;
        v = avio_r8(pb);
        n--;
        if (state == 0x000001) {
            state = ((state << 8) | v) & 0xffffff;
            val   = state;
            goto found;
        }
        state = ((state << 8) | v) & 0xffffff;
    }
    val = -1;

found:
    *header_state = state;
    *size_ptr     = n;
    return val;
}
/*
static void print_pci(uint8_t *buf) {
    uint32_t startpts = AV_RB32(buf + 0x0d);
    uint32_t endpts   = AV_RB32(buf + 0x11);
    uint8_t hours     = ((buf[0x19] >> 4) * 10) + (buf[0x19] & 0x0f);
    uint8_t mins      = ((buf[0x1a] >> 4) * 10) + (buf[0x1a] & 0x0f);
    uint8_t secs      = ((buf[0x1b] >> 4) * 10) + (buf[0x1b] & 0x0f);

    printf("startpts %u endpts %u %d:%d:%d\n",
           startpts, endpts, hours, mins, secs);
}

static void print_dsi(uint8_t *buf) {
    uint16_t vob_idn  = buf[6 * 4] << 8 | buf[6 * 4 + 1];
    uint8_t vob_c_idn = buf[6 * 4 + 2];
    uint8_t hours     = ((buf[0x1d] >> 4) * 10) + (buf[0x1d] & 0x0f);
    uint8_t mins      = ((buf[0x1e] >> 4) * 10) + (buf[0x1e] & 0x0f);
    uint8_t secs      = ((buf[0x1f] >> 4) * 10) + (buf[0x1f] & 0x0f);
    printf("vob idn %d c_idn %d %d:%d:%d\n",
           vob_idn, vob_c_idn, hours, mins, secs);
}
*/
void parse_nav_pack(AVIOContext *pb, int32_t *header_state, VOBU *vobu)
{
	
	static int last_cell_seen=-1;
	static dvd_time_t last_cell_c_eltm;
	
    int size = MAX_SYNC_SIZE, startcode, len;
    uint8_t pci[NAV_PCI_SIZE];
    uint8_t dsi[NAV_DSI_SIZE];

    avio_read(pb, pci, NAV_PCI_SIZE);

    startcode = find_next_start_code(pb, &size, header_state);
    len = avio_rb16(pb);
    if (startcode != PRIVATE_STREAM_2 ||
        len != NAV_DSI_SIZE) {
        return;
    }
    avio_read(pb, dsi, NAV_DSI_SIZE);

    navRead_PCI(&vobu->pci, pci + 1);
    navRead_DSI(&vobu->dsi, dsi + 1);

    // navPrint_PCI(&vobu->pci);
    // navPrint_DSI(&vobu->dsi);
    vobu->vob_id  = vobu->dsi.dsi_gi.vobu_vob_idn;
    vobu->cell_id = vobu->dsi.dsi_gi.vobu_c_idn;
	
	
	
	
	
	if (last_cell_seen!=-1 && last_cell_seen!=vobu->cell_id) {
	
		printf("cell_id=%04d secs %02X:%02X:%02X.%02X \n ",  last_cell_seen, 
			last_cell_c_eltm.hour,
			last_cell_c_eltm.minute,
			last_cell_c_eltm.second,
			last_cell_c_eltm.frame_u);
		
	}
	
	last_cell_c_eltm=vobu->dsi.dsi_gi.c_eltm;
	last_cell_seen=vobu->cell_id;
}

int find_vobu(AVIOContext *pb, VOBU *vobus, int i)
{
    int size = MAX_SYNC_SIZE, startcode;
    int32_t header_state;

redo:
    header_state = 0xff;
    size = MAX_SYNC_SIZE;
    startcode = find_next_start_code(pb, &size, &header_state);
    if (startcode < 0) {
        if (!pb->eof_reached) {
            av_log(NULL, AV_LOG_ERROR, "BOGUS STARTCODE, skipping\n");
            goto redo;
        } else
            return AVERROR_EOF;
    }

    if (startcode == PACK_START_CODE ||
        startcode == SYSTEM_HEADER_START_CODE)
        goto redo;

    if (startcode == PRIVATE_STREAM_2) {
        int len = avio_rb16(pb);
        if (len == NAV_PCI_SIZE) {
            int64_t pos = avio_tell(pb) - 44;
            parse_nav_pack(pb, &header_state, &vobus[i]);
            if (!vobus[i].vob_id)
                goto redo;
            vobus[i].start_sector = pos / 2048;
            vobus[i].start        = pos;
            if (i) {
                vobus[i - 1].end        = vobus[i].start;
                vobus[i - 1].end_sector = vobus[i].start_sector;
            }
            return 0;
         } else {
            goto redo;
        }
    } else {
        goto redo;
    }
}

int populate_vobs(VOBU **v, const char *filename, int index)
{
    AVIOContext *in = NULL;
    static VOBU *vobus = NULL;
    int ret, i = 0;
    static int size = 1;
    static int previous_vobus_found = 0;
	
    int64_t end;

	if (index==1) {
		size = 1;
		previous_vobus_found = 0;	
		vobus = NULL;
	}
	
	printf("Analysing VOB file %s...\n",filename);
	
	i=previous_vobus_found;
	
    ret = avio_open(&in, filename, AVIO_FLAG_READ);

    if (ret < 0) {
        char errbuf[128];
        av_strerror(ret, errbuf, sizeof(errbuf));
        av_log(NULL, AV_LOG_ERROR, "Cannot open %s: %s",
               filename, errbuf);
        return -1;
    }

    end = avio_size(in);

	
	//100000 * sizeof(VOBU)
	
    if (av_reallocp_array(&vobus, size, sizeof(VOBU)) < 0)
        return -1;

    while (!find_vobu(in, vobus, i)) {
        if (i) {
            if (vobus[i - 1].vob_id != vobus[i].vob_id ||
                vobus[i - 1].cell_id != vobus[i].cell_id) {
                vobus[i - 1].next = 0x3fffffff;
            } else {
                vobus[i - 1].next = vobus[i - 1].end_sector -
                                    vobus[i - 1].start_sector;
            }
            av_log(NULL, AV_LOG_DEBUG, "%d Values %d vs %d %d vs %d\n",
                   i - 1,
                   vobus[i - 1].vob_id, vobus[i].vob_id,
                   vobus[i - 1].cell_id, vobus[i].cell_id);
				   
			/*printf("%d Values %d vs %d %d vs %d\n",
                   i - 1,
                   vobus[i - 1].vob_id, vobus[i].vob_id,
                   vobus[i - 1].cell_id, vobus[i].cell_id);fflush(stdout);*/
        }
        if ( ((++i)) >= size - 1) {
            size *= 2;
            if (av_reallocp_array(&vobus, size, sizeof(VOBU)) < 0)
                return -1;
        }
    }

    if (i) {
        vobus[i - 1].end        = end;
        vobus[i - 1].end_sector = end / 2048;
        vobus[i - 1].next       = 0x3fffffff;

        if (i != 1)
            vobus[i].start_sector = -1; // Guard

		*v = vobus;

    } else {
        av_log(NULL, AV_LOG_ERROR, "Empty %s",
               filename);
        return -1;
    }

    avio_close(in);

	previous_vobus_found = i;
	
	printf("... VOBU array size=%d\n",size);
	
    return i;
}

int populate_all_vobs(VOBU **v, const char *path)
{
	char title[1024];
	int nb_vobus;
	int total_vobus=0;
		
	for(int idx=1;idx<5;idx++) {
	
		snprintf(title, sizeof(title), "%s/VIDEO_TS/VTS_01_%d.VOB", path, idx);

		if ((nb_vobus = populate_vobs(v, title, idx)) < 0)
		{
			printf("... not found %d VOBUs\n", nb_vobus);
			//exit(-1);
		}
		else {
			printf("... found %d VOBUs\n", nb_vobus);
			total_vobus=nb_vobus;
		}

		fflush(stdout);
	}		

	printf(". found a total of %d VOBUs\n", total_vobus);
	
	return total_vobus;
}

int populate_cells(CELL **c, VOBU *vobus, int nb_vobus)
{
    int i, j = 0;
    CELL *cell;

	printf("populate_cells starting n vobus:%u\n ",nb_vobus);
			
	
    // FIXME lazy
    cell = av_mallocz(nb_vobus * sizeof(CELL));

    if (!cell)
        return AVERROR(ENOMEM);

    for (i = 1; i <= nb_vobus; i++) {
		
		//printf ("     populate cell    -----    i:%d cell id:%d\n,", i-1, vobus[i - 1].cell_id);
		
        if ((vobus[i - 1].cell_id != vobus[i].cell_id ||
            vobus[i - 1].vob_id != vobus[i].vob_id
			
			)
			&& (vobus[i - 1].cell_id != 0 && vobus[i - 1].cell_id != 1 && vobus[i - 1].cell_id != 255))
			{
            if (j) {
                cell[j].start_sector   = cell[j - 1].last_sector + 1;
				
				cell[j-1].duration = vobus[i-1].dsi.dsi_gi.c_eltm;
				
            }

            cell[j].vob_id        = vobus[i - 1].vob_id;
            cell[j].cell_id       = vobus[i - 1].cell_id;
            cell[j].last_vobu_start_sector = vobus[i - 1].start_sector;
			
			cell[j].isOrphan=1;
			
			printf("    populate_cells i:%u j:%u   secs %02X:%02X:%02X.%02X \n ",i,j,
			cell[j].duration.hour,
			cell[j].duration.minute,
			cell[j].duration.second,
			cell[j].duration.frame_u);	
			
			
            cell[j++].last_sector = vobus[i - 1].end_sector - 1;
        }
    }

    *c = cell;

	printf("populate_cells ending n cells:%u\n ", j);
	
    return j;
}


int get_first_orphan_cell_id(CELL *cells, int nb_vobus)
{
    int i = 0;

    for (i = 1; i <= nb_vobus; i++) {
        if (cells[i-1].isOrphan)
			return cells[i-1].cell_id;
	}

	return 0;
}