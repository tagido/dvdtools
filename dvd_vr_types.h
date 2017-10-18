#ifndef DVD_VR_TYPES_H
#define DVD_VR_TYPES_H

#include <dvdread/ifo_read.h>


/**
* Video Manager Information Management Table.
*/
typedef struct {
	char     vrm_identifier[12];
	uint32_t vmg_last_sector;
	uint8_t  zero_1[12];
	uint32_t vmgi_last_sector;
	uint8_t  zero_2;
	uint8_t  specification_version;
	uint32_t vmg_category;
	uint16_t vmg_nr_of_volumes;
	uint16_t vmg_this_volume_nr;
	uint8_t  disc_side;
	uint8_t  zero_3[19];
	uint16_t vmg_nr_of_title_sets;  /* Number of VTSs. */
	char     provider_identifier[32];
	uint64_t vmg_pos_code;
	uint8_t  zero_4[24];
	uint32_t vmgi_last_byte;
	uint32_t first_play_pgc;
	uint8_t  zero_5[56];
	uint32_t vmgm_vobs;             /* sector */
	uint32_t tt_srpt;               /* sector */
	uint32_t vmgm_pgci_ut;          /* sector */
	uint32_t ptl_mait;              /* sector */
	uint32_t vts_atrt;              /* sector */
	uint32_t txtdt_mgi;             /* sector */
	uint32_t vmgm_c_adt;            /* sector */
	uint32_t vmgm_vobu_admap;       /* sector */
	uint8_t  zero_6[32];

	video_attr_t vmgm_video_attr;
	uint8_t  zero_7;
	uint8_t  nr_of_vmgm_audio_streams; /* should be 0 or 1 */
	audio_attr_t vmgm_audio_attr;
	audio_attr_t zero_8[7];
	uint8_t  zero_9[17];
	uint8_t  nr_of_vmgm_subp_streams; /* should be 0 or 1 */
	subp_attr_t  vmgm_subp_attr;
	subp_attr_t  zero_10[27];  /* XXX: how much 'padding' here? */
} ATTRIBUTE_PACKED vrm_mat_t;


typedef struct {
	//dvd_file_t *file;

	vrm_mat_t     *vrm_mat;
	recording_table_t      *recordings_srpt;


} dvd_vr_video_rm_handle_t;

#endif