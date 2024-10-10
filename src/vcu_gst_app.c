/*********************************************************************
 * Copyright (C) 2017-2021 Xilinx, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 ********************************************************************/
#include <netinet/in.h>
#include <arpa/inet.h>
#include "configs.h"
#include "vcu_gst_app.h"
#include "DBS.h"
#include "tseTypes.h" //TODO: ODELYA - MAX_NUM_STREAMERS may not match NUM_GST_CHANNELS - need to fix

App app_data[NUM_GST_CHANNELS] = {};
gint standalone_channal_number = (-1);


/**
 * \internal_start
 * \brief an explanation on how to use vcu-gst-app application
 *
 *
 * \sdd_start
 * <B>CSU Requirements and Constraints:</B>\n
 * \id_start usage \id_end
 * \csu_start  \csu_end\n
 * \req_start \req_end\n
 *
 * <B>Algorithm & logic flow:</B>\n
 * - print instructions
 * \sdd_end
 *
 * <B>Error handling & limitations:</B>\n
 *
 * \param NONE.
 *
 * \return NONE.
 * \internal_end
 */
void
usage () {
    g_print ("minimum 2 arguments are required:\n"
    	"./vcu_gst_app <channel> <cfg file>\n"
    	"./vcu_gst_app 0 ./input.cfg\n");
}

/**
 * \internal_start
 * \brief Skip white spaces of a string
 *
 *
 * \sdd_start
 * <B>CSU Requirements and Constraints:</B>\n
 * \id_start skip_whitespace \id_end
 * \csu_start  \csu_end\n
 * \req_start \req_end\n
 *
 * <B>Algorithm & logic flow:</B>\n
 * - skip the white spaces
 * \sdd_end
 *
 * <B>Error handling & limitations:</B>\n
 *
 * \param gchar* cursor - the string to skip whitespaces
 *
 * \return static gchar* - the output string
 * \internal_end
 */
static gchar*
skip_whitespace(gchar* cursor) {
    gchar *new = NULL;
    if (!cursor)
        return NULL;
    if (*cursor == '\0')
        return NULL;
    while (*cursor == '\t' || *cursor == '\r' || *cursor == ' ' || *cursor == ':') {
        ++(cursor);
    }
    new = strtok (cursor, "\n");
    return new;
}

/**
 * \internal_start
 * \brief Check if we reach the exit criteria
 *
 *
 * \sdd_start
 * <B>CSU Requirements and Constraints:</B>\n
 * \id_start meet_exit_criterion \id_end
 * \csu_start  \csu_end\n
 * \req_start \req_end\n
 *
 * <B>Algorithm & logic flow:</B>\n
 * - check if we reach the exit criteria
 * \sdd_end
 *
 * <B>Error handling & limitations:</B>\n
 *
 * \param unsigned int channel - the required channel
 *
 * \return gboolean - whether or not to exit
 * \internal_end
 */
gboolean
meet_exit_criterion (unsigned int channel) {
	if((channel >= 0) && (channel < NUM_GST_CHANNELS)) {
		App* pApp = &app_data[channel];
		gchar *ptr = pApp->line;
		if (ptr) {
			ptr = skip_whitespace (ptr);
			if ((ptr && !strncasecmp(ptr, EXIT, strlen (EXIT))) || feof(pApp->file)) {
				return TRUE;
			}
		}
	}
    return FALSE;
}

/**
 * \internal_start
 * \brief Get value of required parameter
 *
 *
 * \sdd_start
 * <B>CSU Requirements and Constraints:</B>\n
 * \id_start get_value \id_end
 * \csu_start  \csu_end\n
 * \req_start \req_end\n
 *
 * <B>Algorithm & logic flow:</B>\n
 * - check if we reach the exit criteria
 * \sdd_end
 *
 * <B>Error handling & limitations:</B>\n
 *
 * \param gchar *ptr - the read line
 * \param const gchar *key - the key of the read line
 *
 * \return gchar* - the required param
 * \internal_end
 */
static gchar*
get_value (gchar *ptr, const gchar *key) {
    ptr += strlen(key);
    ptr = skip_whitespace (ptr);
    return ptr;
}

/**
 * \internal_start
 * \brief Get common configuration for the required channel
 *
 *
 * \sdd_start
 * <B>CSU Requirements and Constraints:</B>\n
 * \id_start get_cmn_config \id_end
 * \csu_start  \csu_end\n
 * \req_start \req_end\n
 *
 * <B>Algorithm & logic flow:</B>\n
 * - read all lines until reach the exit criteria:
 * 	-	update channel configuration
 * \sdd_end
 *
 * <B>Error handling & limitations:</B>\n
 *
 * \param guint channel - the required channel
 *
 * \return NONE.
 * \internal_end
 */
void
get_cmn_config (guint channel) {
    gint temp =0;
    gchar *ptr = NULL;

    if((channel >= 0) && (channel < NUM_GST_CHANNELS)) {
	App* pApp = &app_data[channel];

		while (!meet_exit_criterion (channel)) {
			fgets (pApp->line, sizeof (pApp->line), pApp->file);
			if ((ptr = strstr (pApp->line, NUM_SRC)) != NULL) {
				ptr = get_value (ptr, NUM_SRC);
				temp = atoi (ptr);
				if (temp >= MIN_NUM_SOURCES && temp <= MAX_NUM_SOURCES) {
					pApp->cmnParam.num_src = temp;
				} else {
					g_print ("Warning!! Num of Input value is incorrect taking default\n");
				}
			} else if ((ptr = strstr (pApp->line, OUTPUT)) != NULL) {
				ptr = get_value (ptr, OUTPUT);
				if (ptr ) {
					if (!strncasecmp (ptr, DP_OUT, strlen (DP_OUT))) {
						pApp->cmnParam.driver_type = DP;
					} else if (!strncasecmp (ptr, HDMI_OUT, strlen (HDMI_OUT)) || !strncasecmp (ptr, HDMI_TX_OUT, strlen (HDMI_TX_OUT))) {
						pApp->cmnParam.driver_type = HDMI_Tx;
					} else if (!strncasecmp (ptr, SDI_OUT, strlen (SDI_OUT)) || !strncasecmp (ptr, SDI_TX_OUT, strlen (SDI_TX_OUT))) {
						pApp->cmnParam.driver_type = SDI_Tx;
					} else {
						g_print ("Warning!! output destination value is incorrect taking default\n");
					}
				} else {
					g_print ("Warning!! Output destination value is incorrect taking default\n");
				}
			} else if ((ptr = strstr (pApp->line, OUTPUT_TYPE)) != NULL) {
				ptr = get_value (ptr, OUTPUT_TYPE);
				if (ptr) {
					if (!strncasecmp (ptr, RECORD_FILE_OUT, strlen (RECORD_FILE_OUT))) {
						pApp->cmnParam.sink_type = RECORD;
					} else if (!strncasecmp (ptr, STREAM_OUT, strlen (STREAM_OUT))) {
						pApp->cmnParam.sink_type = STREAM;
					} else if (!strncasecmp (ptr, DISPLAY_OUT, strlen (DISPLAY_OUT))) {
						pApp->cmnParam.sink_type = DISPLAY;
					} else if (!strncasecmp (ptr, SPLIT_SCREEN_OUT, strlen (SPLIT_SCREEN_OUT))) {
						pApp->cmnParam.sink_type = SPLIT_SCREEN;
					} else {
						g_print ("Warning!! Output type value is incorrect taking default\n");
					}
				} else {
					g_print ("Warning!! Output type value is incorrect taking default\n");
				}
			} else if ((ptr = strstr (pApp->line, FRAME_RATE)) != NULL) {
				ptr = get_value (ptr, FRAME_RATE);
				ptr = skip_whitespace (ptr);
				if (ptr) {
					pApp->cmnParam.frame_rate = atoi (ptr);
				} else {
					g_print ("Warning!! Frame rate is incorrect taking default\n");
				}
			}
		}
    }
}

/**
 * \internal_start
 * \brief Get input configuration for the required channel
 *
 *
 * \sdd_start
 * <B>CSU Requirements and Constraints:</B>\n
 * \id_start get_input_config \id_end
 * \csu_start  \csu_end\n
 * \req_start \req_end\n
 *
 * <B>Algorithm & logic flow:</B>\n
 * - read all lines until reach the exit criteria:
 * 	-	update channel configuration
 * \sdd_end
 *
 * <B>Error handling & limitations:</B>\n
 *
 * \param guint channel - the required channel
 *
 * \return NONE.
 * \internal_end
 */
void
get_input_config (guint channel) {
    gchar *ptr = NULL;
    guint tmp, cnt =1;

    if((channel >= 0) && (channel < NUM_GST_CHANNELS)) {
		App* pApp = &app_data[channel];

		while (!meet_exit_criterion (channel)) {
			ptr = fgets (pApp->line, sizeof (pApp->line), pApp->file);
			if ((ptr = strstr (pApp->line, INPUT_NUM)) != NULL) {
				ptr = get_value (ptr, INPUT_NUM);
				tmp = atoi (ptr);
				if (tmp >= MIN_NUM_SOURCES && tmp <= MAX_NUM_SOURCES) {
					cnt = tmp;
				} else {
					cnt = 1;
					g_print ("Warning!! Input Num value is incorrect taking default =%d\n", cnt);
				}
			} else if ((ptr = strstr (pApp->line, ACCELERATOR_FILTER)) != NULL) {
				ptr = get_value (ptr, ACCELERATOR_FILTER);
				if (ptr) {
					if (!strncasecmp (ptr, TRUE_VAL, strlen (TRUE_VAL))) {
						pApp->ipParam[cnt-1].accelerator = TRUE;
					} else if (!strncasecmp (ptr, FALSE_VAL, strlen (FALSE_VAL ))) {
						pApp->ipParam[cnt-1].accelerator = FALSE;
					} else {
						g_print ("Warning!! accelerator flag is incorrect taking default\n");
					}
				} else {
					g_print ("Warning!! accelerator flag is incorrect taking default\n");
				}
			} else if ((ptr = strstr (pApp->line, ENABLE_SCD)) != NULL) {
				ptr = get_value (ptr, ENABLE_SCD);
				if (ptr) {
					if (!strncasecmp (ptr, TRUE_VAL, strlen (TRUE_VAL))) {
						pApp->ipParam[cnt-1].enable_scd = TRUE;
					} else if (!strncasecmp (ptr, FALSE_VAL, strlen (FALSE_VAL ))) {
						pApp->ipParam[cnt-1].enable_scd = FALSE;
					} else {
						g_print ("Warning!! enable_scd flag is incorrect taking default\n");
					}
				} else {
					g_print ("Warning!! enable_scd flag is incorrect taking default\n");
				}
			} else if ((ptr = strstr (pApp->line, ENABLE_ROI)) != NULL) {
				ptr = get_value (ptr, ENABLE_ROI);
				if (ptr) {
					if (!strncasecmp (ptr, TRUE_VAL, strlen (TRUE_VAL))) {
						pApp->ipParam[cnt-1].enable_roi = TRUE;
					} else if (!strncasecmp (ptr, FALSE_VAL, strlen (FALSE_VAL ))) {
						pApp->ipParam[cnt-1].enable_roi = FALSE;
					} else {
						g_print ("Warning!! enable_roi flag is incorrect taking default\n");
					}
				} else {
					g_print ("Warning!! enable_roi flag is incorrect taking default\n");
				}
			} else if ((ptr = strstr (pApp->line, ENABLE_LLP2)) != NULL) {
				ptr = get_value (ptr, ENABLE_LLP2);
				if (ptr) {
					if (!strncasecmp (ptr, TRUE_VAL, strlen (TRUE_VAL))) {
						pApp->ipParam[cnt-1].enable_llp2 = TRUE;
					} else if (!strncasecmp (ptr, FALSE_VAL, strlen (FALSE_VAL ))) {
						pApp->ipParam[cnt-1].enable_llp2 = FALSE;
					} else {
						g_print ("Warning!! enable_llp2 flag is incorrect taking default\n");
					}
				} else {
					g_print ("Warning!! enable_llp2 flag is incorrect taking default\n");
				}
			} else if ((ptr = strstr (pApp->line, INPUT_TYPE)) != NULL) {
				ptr = get_value (ptr, INPUT_TYPE);
				if (ptr) {
					if (!strncasecmp (ptr, HDMI_2_INPUT, strlen (HDMI_2_INPUT)) || !strncasecmp (ptr, HDMI_RX_2_INPUT, strlen(HDMI_RX_2_INPUT))) {
						pApp->ipParam[cnt-1].device_type = HDMI_2;
						pApp->ipParam[cnt-1].src_type = LIVE_SRC;
					} else if (!strncasecmp (ptr, HDMI_3_INPUT, strlen (HDMI_3_INPUT)) || !strncasecmp (ptr, HDMI_RX_3_INPUT, strlen(HDMI_RX_3_INPUT))) {
						pApp->ipParam[cnt-1].device_type = HDMI_3;
						pApp->ipParam[cnt-1].src_type = LIVE_SRC;
					} else if (!strncasecmp (ptr, HDMI_4_INPUT, strlen (HDMI_4_INPUT)) || !strncasecmp (ptr, HDMI_RX_4_INPUT, strlen(HDMI_RX_4_INPUT))) {
						pApp->ipParam[cnt-1].device_type = HDMI_4;
						pApp->ipParam[cnt-1].src_type = LIVE_SRC;
					} else if (!strncasecmp (ptr, HDMI_5_INPUT, strlen (HDMI_5_INPUT)) || !strncasecmp (ptr, HDMI_RX_5_INPUT, strlen(HDMI_RX_5_INPUT))) {
						pApp->ipParam[cnt-1].device_type = HDMI_5;
						pApp->ipParam[cnt-1].src_type = LIVE_SRC;
					} else if (!strncasecmp (ptr, HDMI_6_INPUT, strlen (HDMI_6_INPUT)) || !strncasecmp (ptr, HDMI_RX_6_INPUT, strlen(HDMI_RX_6_INPUT))) {
						pApp->ipParam[cnt-1].device_type = HDMI_6;
						pApp->ipParam[cnt-1].src_type = LIVE_SRC;
					} else if (!strncasecmp (ptr, HDMI_7_INPUT, strlen (HDMI_7_INPUT)) || !strncasecmp (ptr, HDMI_RX_7_INPUT, strlen(HDMI_RX_7_INPUT))) {
						pApp->ipParam[cnt-1].device_type = HDMI_7;
						pApp->ipParam[cnt-1].src_type = LIVE_SRC;
					} else if (!strncasecmp (ptr, TPG_2_INPUT, strlen (TPG_2_INPUT))) {
						pApp->ipParam[cnt-1].device_type = TPG_2;
						pApp->ipParam[cnt-1].src_type = LIVE_SRC;
					} else if (!strncasecmp (ptr, TPG_1_INPUT, strlen (TPG_1_INPUT))) {
						pApp->ipParam[cnt-1].device_type = TPG_1;
						pApp->ipParam[cnt-1].src_type = LIVE_SRC;
					} else if (!strncasecmp (ptr, HDMI_INPUT, strlen (HDMI_INPUT)) || !strncasecmp (ptr, HDMI_RX_INPUT, strlen (HDMI_RX_INPUT))) {
						pApp->ipParam[cnt-1].device_type = HDMI_1;
						pApp->ipParam[cnt-1].src_type = LIVE_SRC;
					} else if (!strncasecmp (ptr, CSI_2_INPUT, strlen (CSI_2_INPUT))) {
						pApp->ipParam[cnt-1].device_type = CSI_2;
						pApp->ipParam[cnt-1].src_type = LIVE_SRC;
					} else if (!strncasecmp (ptr, CSI_3_INPUT, strlen (CSI_3_INPUT))) {
						pApp->ipParam[cnt-1].device_type = CSI_3;
						pApp->ipParam[cnt-1].src_type = LIVE_SRC;
					} else if (!strncasecmp (ptr, CSI_4_INPUT, strlen (CSI_4_INPUT))) {
						pApp->ipParam[cnt-1].device_type = CSI_4;
						pApp->ipParam[cnt-1].src_type = LIVE_SRC;
					} else if (!strncasecmp (ptr, MIPI_INPUT, strlen (MIPI_INPUT)) || !strncasecmp (ptr, CSI_INPUT, strlen (CSI_INPUT)) ||\
							   !strncasecmp (ptr, MIPI_CSI_INPUT, strlen (MIPI_CSI_INPUT))) {
						pApp->ipParam[cnt-1].device_type = CSI;
						pApp->ipParam[cnt-1].src_type = LIVE_SRC;
					} else if (!strncasecmp (ptr, SDI_INPUT, strlen (SDI_INPUT)) || !strncasecmp (ptr, SDI_RX_INPUT, strlen (SDI_RX_INPUT))) {
						pApp->ipParam[cnt-1].device_type = SDI;
						pApp->ipParam[cnt-1].src_type = LIVE_SRC;
					} else if (!strncasecmp (ptr, FILE_INPUT, strlen (FILE_INPUT))) {
						pApp->ipParam[cnt-1].src_type = FILE_SRC;
					} else if (!strncasecmp (ptr, STREAMING_INPUT, strlen (STREAMING_INPUT))) {
						pApp->ipParam[cnt-1].src_type = STREAMING_SRC;
					} else {
						g_print ("Warning!! input type value is incorrect taking default\n");
					}
				} else {
					g_print ("Warning!! input type value is incorrect taking default\n");
				}
			} else if ((ptr = strstr (pApp->line, URI)) != NULL) {
				ptr = get_value (ptr, URI);
				if (ptr) {
					pApp->ipParam[cnt-1].uri = g_strdup (ptr);
				} else {
					g_print ("Warning!! file Uri is null\n");
				}
			} else if ((ptr = strstr (pApp->line, FORMAT)) != NULL) {
				ptr = get_value (ptr, FORMAT);
				if (ptr) {
					if (!strncasecmp (ptr, NV12_FORMAT, strlen (NV12_FORMAT))) {
						pApp->ipParam[cnt-1].format = NV12;
					} else if (!strncasecmp (ptr, NV16_FORMAT, strlen (NV16_FORMAT))) {
						pApp->ipParam[cnt-1].format = NV16;
					} else if (!strncasecmp (ptr, XV15_FORMAT, strlen (XV15_FORMAT))) {
						pApp->ipParam[cnt-1].format = XV15;
					} else if (!strncasecmp (ptr, XV20_FORMAT, strlen (XV20_FORMAT))) {
						pApp->ipParam[cnt-1].format = XV20;
					} else {
						g_print ("Warning!! Format value is incorrect taking default\n");
					}
				} else {
					g_print ("Warning!! Format value is incorrect taking default\n");
				}
			} else if ((ptr = strstr (pApp->line, RAW)) != NULL) {
				ptr = get_value (ptr, RAW);
				if (ptr ) {
					if (!strncasecmp (ptr, TRUE_VAL, strlen (TRUE_VAL))) {
						pApp->ipParam[cnt-1].raw = TRUE;
					} else if (!strncasecmp (ptr, FALSE_VAL, strlen (FALSE_VAL ))) {
						pApp->ipParam[cnt-1].raw = FALSE;
					} else {
						g_print ("Warning!! Raw flag value is incorrect taking default\n");
					}
				} else {
					g_print ("Warning!! Raw flag value is incorrect taking default\n");
				}
			} else if ((ptr = strstr (pApp->line, WIDTH)) != NULL) {
				ptr = get_value (ptr, WIDTH);
				if (ptr) {
					pApp->ipParam[cnt-1].width = atoi (ptr);
				} else {
					g_print ("Warning!! Width value is incorrect taking default\n");
				}
			} else if ((ptr = strstr (pApp->line, HEIGHT)) != NULL) {
				ptr = get_value (ptr, HEIGHT);
				if (ptr) {
					pApp->ipParam[cnt-1].height = atoi (ptr);
				} else {
					g_print ("Warning!! Height value is incorrect taking default\n");
				}
			} else if ((ptr = strstr (pApp->line, RELATIVE_QP)) != NULL) {
				ptr = get_value (ptr, RELATIVE_QP);
				if (ptr) {
					pApp->ipParam[cnt-1].relative_qp = atoi (ptr);
				} else {
					g_print ("Warning!! Relative qp value is incorrect taking default\n");
				}
			}
		}
    }
}

/**
 * \internal_start
 * \brief Set encoder preset configuration for the required channel
 *
 *
 * \sdd_start
 * <B>CSU Requirements and Constraints:</B>\n
 * \id_start set_preset_config \id_end
 * \csu_start  \csu_end\n
 * \req_start \req_end\n
 *
 * <B>Algorithm & logic flow:</B>\n
 * - read all lines until reach the exit criteria:
 * 	-	update channel configuration
 * \sdd_end
 *
 * <B>Error handling & limitations:</B>\n
 *
 * \param guint channel - the required channel
 * \param gchar *ptr - pointer to lines to parse
 * \param guint index - the encoder source number
 * 						in case there are more then 1 source (in our case there is only 1 source per channel)
 *
 * \return gboolean - whether or not the preset config were set
 * \internal_end
 */
gboolean
set_preset_config (guint channel, gchar *ptr, guint index) {
	if((channel >= 0) && (channel < NUM_GST_CHANNELS)) {
		App* pApp = &app_data[channel];
		vgst_enc_params *encParam = &pApp->encParam[index-1];

		if (!strncasecmp (ptr, HEVC_HIGH, strlen (HEVC_HIGH))) {
			encParam->bitrate = HIGH_BITRATE;
			encParam->enc_type = HEVC;
		} else if (!strncasecmp (ptr, HEVC_MEDIUM, strlen (HEVC_MEDIUM))) {
			encParam->bitrate = MEDIUM_BITRATE;
			encParam->enc_type = HEVC;
		} else if (!strncasecmp (ptr, HEVC_LOW, strlen (HEVC_LOW))) {
			encParam->bitrate = LOW_BITRATE;
			encParam->enc_type = HEVC;
		} else if (!strncasecmp (ptr, AVC_HIGH, strlen (AVC_HIGH))) {
			encParam->bitrate = HIGH_BITRATE;
			encParam->enc_type = AVC;
		} else if (!strncasecmp (ptr, AVC_MEDIUM, strlen (AVC_MEDIUM))) {
			encParam->bitrate = MEDIUM_BITRATE;
			encParam->enc_type = AVC;
		} else if (!strncasecmp (ptr, AVC_LOW, strlen (AVC_LOW))) {
			encParam->bitrate = LOW_BITRATE;
			encParam->enc_type = AVC;
		} else if (!strncasecmp (ptr, AVC_ENC_NAME, strlen (AVC_ENC_NAME))) {
			g_print("Preset %s\n",ptr);
			encParam->bitrate = (guint)atoi(ptr+strlen (AVC_ENC_NAME)+1);
			g_print("bitrate %u\n",encParam->bitrate);
			encParam->enc_type = AVC;
		} else if (!strncasecmp (ptr, HEVC_ENC_NAME, strlen (AVC_ENC_NAME))) {
			g_print("Preset %s\n",ptr);
			encParam->bitrate = (guint)atoi(ptr+strlen (HEVC_ENC_NAME)+1);
			g_print("bitrate %u\n",encParam->bitrate);
			encParam->enc_type = HEVC;
		} else {
			return FALSE;
		}
		encParam->b_frame = DEFAULT_B_FRAME;
		encParam->enable_l2Cache = TRUE;
		encParam->gop_len = DEFAULT_GOP_LEN;
		if (encParam->enc_type == HEVC)
			encParam->profile = MAIN_PROFILE;
		else
			encParam->profile = HIGH_PROFILE;
		encParam->qp_mode = AUTO;
		encParam->rc_mode = CBR;
		encParam->slice = DEFAULT_NUM_SLICE;
		encParam->gop_mode = BASIC;
		encParam->filler_data = TRUE;
		encParam->low_bandwidth = FALSE;
		encParam->latency_mode = NORMAL_LATENCY;
		encParam->gdr_mode = GDR_MODE_DISABLED;
		encParam->hlg_sdr_compatible = FALSE;
		if (encParam->enc_type == HEVC)
			encParam->entropy_mode = ENTROPY_MODE_CABAC;
		else
			encParam->entropy_mode = ENTROPY_MODE_CAVLC;
		encParam->max_picture_size = FALSE;
	}
    return TRUE;
}

/**
 * \internal_start
 * \brief Get encoder configuration for the required channel
 *
 *
 * \sdd_start
 * <B>CSU Requirements and Constraints:</B>\n
 * \id_start get_encoder_config \id_end
 * \csu_start  \csu_end\n
 * \req_start \req_end\n
 *
 * <B>Algorithm & logic flow:</B>\n
 * - read all lines until reach the exit criteria:
 * 	-	update channel configuration
 * \sdd_end
 *
 * <B>Error handling & limitations:</B>\n
 *
 * \param guint channel - the required channel
 *
 * \return NONE.
 * \internal_end
 */
void
get_encoder_config (guint channel) {
    gchar *ptr = NULL;
    guint tmp = 0, cnt = 0;

    if((channel >= 0) && (channel < NUM_GST_CHANNELS)) {
		App* pApp = &app_data[channel];

		while (!meet_exit_criterion (channel)) {
			ptr = fgets (pApp->line, sizeof (pApp->line), pApp->file);
			if ((ptr = strstr (pApp->line, ENCODER_NUM)) != NULL) {
				ptr = get_value (ptr, ENCODER_NUM);
				tmp = atoi (ptr);
				if (tmp >= MIN_NUM_SOURCES && tmp <= MAX_NUM_SOURCES) {
					cnt = tmp;
				} else {
					cnt = MIN_NUM_SOURCES;
					g_print ("Warning!! Encoder Num value is incorrect taking default =%d\n", cnt);
				}
			} else if ((ptr = strstr (pApp->line, PRESET)) != NULL) {
				ptr = get_value (ptr, PRESET);
				if (ptr) {
					if (strncasecmp (ptr, CUSTOM, strlen (CUSTOM))) {
						if (!set_preset_config (channel, ptr, cnt)) {
							g_print ("Warning!! Preset value is incorrect taking custom as default\n");
						}
					}
				} else {
					g_print ("Warning!! Preset value is incorrect taking custom as default\n");
				}
			} else if ((ptr = strstr (pApp->line, ENCODER_NAME)) != NULL) {
				ptr = get_value (ptr, ENCODER_NAME);
				if (ptr) {
					if (!strncasecmp (ptr, H264_ENC_NAME, strlen (H264_ENC_NAME)) || !strncasecmp (ptr, AVC_ENC_NAME, strlen (AVC_ENC_NAME))) {
						pApp->encParam[cnt-1].enc_type = AVC;
					} else if (!strncasecmp (ptr, H265_ENC_NAME, strlen (H265_ENC_NAME)) || !strncasecmp (ptr, HEVC_ENC_NAME, strlen (HEVC_ENC_NAME))) {
						pApp->encParam[cnt-1].enc_type = HEVC;
					} else {
						g_print ("Warning!! Encoder Name is incorrect taking default\n");
					}
				} else {
					g_print ("Warning!! Encoder Name is incorrect taking default\n");
				}
			} else if ((ptr = strstr (pApp->line, PROFILE)) != NULL) {
				ptr = get_value (ptr, PROFILE);
				if (ptr) {
					if (!strncasecmp (ptr, BASE_PROF, strlen (BASE_PROF))) {
						pApp->encParam[cnt-1].profile = BASELINE_PROFILE;
					} else if (!strncasecmp (ptr, MAIN_PROF, strlen (MAIN_PROF))) {
						pApp->encParam[cnt-1].profile = MAIN_PROFILE;
					} else if (!strncasecmp (ptr, HIGH_PROF, strlen (HIGH_PROF))) {
						pApp->encParam[cnt-1].profile = HIGH_PROFILE;
					} else {
						g_print ("Warning!! profile value is wrong taking default\n");
					}
				} else {
					g_print ("Warning!! profile value is wrong taking default\n");
				}
			} else if ((ptr = strstr (pApp->line, QP_VALUE)) != NULL) {
				ptr = get_value (ptr, QP_VALUE);
				if (ptr) {
					if (!strncasecmp (ptr, UNIFORM_QP, strlen (UNIFORM_QP))) {
						pApp->encParam[cnt-1].qp_mode = UNIFORM;
					} else if (!strncasecmp (ptr, AUTO_QP, strlen (AUTO_QP))) {
						pApp->encParam[cnt-1].qp_mode = AUTO;
					} else if (!strncasecmp (ptr, ROI_QP, strlen (ROI_QP))) {
						pApp->encParam[cnt-1].qp_mode = ROI;
					} else {
						g_print ("Warning!! qp_mode value is wrong taking default\n");
					}
				} else {
					g_print ("Warning!! qp_mode value is wrong taking default\n");
				}
			} else if ((ptr = strstr (pApp->line, RATE_CONTRL)) != NULL) {
				ptr = get_value (ptr, RATE_CONTRL);
				if (ptr) {
					if (!strncasecmp (ptr, CBR_RC, strlen (CBR_RC))) {
						pApp->encParam[cnt-1].rc_mode = CBR;
					} else if (!strncasecmp (ptr, VBR_RC, strlen (VBR_RC))) {
						pApp->encParam[cnt-1].rc_mode = VBR;
					} else if (!strncasecmp (ptr, LOWLATENCY_RC, strlen (LOWLATENCY_RC))) {
						pApp->encParam[cnt-1].rc_mode = LOW_LATENCY;
					} else {
						g_print ("Warning!! rc_mode is wrong taking default\n");
					}
				} else {
					g_print ("Warning!! rc_mode is wrong taking default\n");
				}
			} else if ((ptr = strstr (pApp->line, BITRATE)) != NULL) {
				ptr = get_value (ptr, BITRATE);
				if (ptr) {
					pApp->encParam[cnt-1].bitrate = atoi (ptr);
				} else {
					g_print ("Warning!! bitrate is wrong taking default\n");
				}
			} else if ((ptr = strstr (pApp->line, FILLER_DATA)) != NULL) {
				ptr = get_value (ptr, FILLER_DATA);
				if (ptr) {
					if (!strncasecmp (ptr, TRUE_VAL, strlen (TRUE_VAL))) {
						pApp->encParam[cnt-1].filler_data = TRUE;
					} else if (!strncasecmp (ptr, FALSE_VAL, strlen (FALSE_VAL))) {
						pApp->encParam[cnt-1].filler_data = FALSE;
					} else {
						g_print ("Warning!! filler_data value is wrong taking default\n");
					}
				} else {
					g_print ("Warning!! filler_data value is wrong taking default\n");
				}
			} else if ((ptr = strstr (pApp->line, LOW_BANDWIDTH)) != NULL) {
				ptr = get_value (ptr, LOW_BANDWIDTH);
				if (ptr) {
					if (!strncasecmp (ptr, TRUE_VAL, strlen (TRUE_VAL))) {
						pApp->encParam[cnt-1].low_bandwidth = TRUE;
					} else if (!strncasecmp (ptr, FALSE_VAL, strlen (FALSE_VAL))) {
						pApp->encParam[cnt-1].low_bandwidth = FALSE;
					} else {
						g_print ("Warning!! low_bandwidth value is wrong taking default\n");
					}
				} else {
					g_print ("Warning!! low_bandwidth value is wrong taking default\n");
				}
			} else if ((ptr = strstr (pApp->line, HLG_SDR_COMPATIBLE)) != NULL) {
				ptr = get_value (ptr, HLG_SDR_COMPATIBLE);
				if (ptr) {
					if (!strncasecmp (ptr, TRUE_VAL, strlen (TRUE_VAL))) {
						pApp->encParam[cnt-1].hlg_sdr_compatible = TRUE;
					} else if (!strncasecmp (ptr, FALSE_VAL, strlen (FALSE_VAL))) {
						pApp->encParam[cnt-1].hlg_sdr_compatible = FALSE;
					} else {
						g_print ("Warning!! hlg_sdr_compatible value is wrong taking default\n");
					}
				} else {
					g_print ("Warning!! hlg_sdr_compatible value is wrong taking default\n");
				}
			} else if ((ptr = strstr (pApp->line, GOP_MODE)) != NULL) {
				ptr = get_value (ptr, GOP_MODE);
				if (ptr) {
					if (!strncasecmp (ptr, BASIC_GOP, strlen (BASIC_GOP))) {
						pApp->encParam[cnt-1].gop_mode = BASIC;
					} else if (!strncasecmp (ptr, LOW_DELAY_P_GOP, strlen (LOW_DELAY_P_GOP))) {
						pApp->encParam[cnt-1].gop_mode = LOW_DELAY_P;
					} else if (!strncasecmp (ptr, LOW_DELAY_B_GOP, strlen (LOW_DELAY_B_GOP))) {
						pApp->encParam[cnt-1].gop_mode = LOW_DELAY_B;
					} else {
						g_print ("Warning!! gop_mode is wrong taking default\n");
					}
				} else {
					g_print ("Warning!! gop_mode is wrong taking default\n");
				}
			} else if ((ptr = strstr (pApp->line, B_FRAMES)) != NULL) {
				ptr = get_value (ptr, B_FRAMES);
				if (ptr) {
					pApp->encParam[cnt-1].b_frame = atoi (ptr);
				} else {
					g_print ("Warning!! b_frame value is wrong taking default\n");
				}
			} else if ((ptr = strstr (pApp->line, LATENCY_MODE)) != NULL) {
				ptr = get_value (ptr, LATENCY_MODE);
				if (ptr) {
					if (!strncasecmp (ptr, NORMAL, strlen (NORMAL))) {
						pApp->encParam[cnt-1].latency_mode = NORMAL_LATENCY;
					} else if (!strncasecmp (ptr, SUB_FRAME, strlen (SUB_FRAME))) {
						pApp->encParam[cnt-1].latency_mode = SUB_FRAME_LATENCY;
					} else {
						g_print ("Warning!! latency_mode is wrong taking default\n");
					}
				} else {
					g_print ("Warning!! latency_mode is wrong taking default\n");
				}
			} else if ((ptr = strstr (pApp->line, SLICE)) != NULL) {
				ptr = get_value (ptr, SLICE);
				if (ptr) {
					pApp->encParam[cnt-1].slice = atoi (ptr);
				} else {
					g_print ("Warning!! slice value is wrong taking default\n");
				}
			} else if ((ptr = strstr (pApp->line, GOP_LENGTH)) != NULL) {
				ptr = get_value (ptr, GOP_LENGTH);
				if (ptr) {
					pApp->encParam[cnt-1].gop_len = atoi (ptr);
				} else {
					g_print ("Warning!! gop_len value is wrong taking default\n");
				}
			} else if ((ptr = strstr (pApp->line, L2_CACHE)) != NULL) {
				ptr = get_value (ptr, L2_CACHE);
				if (ptr) {
					if (!strncasecmp (ptr, TRUE_VAL, strlen (TRUE_VAL))) {
						pApp->encParam[cnt-1].enable_l2Cache = TRUE;
					} else if (!strncasecmp (ptr, FALSE_VAL, strlen (FALSE_VAL))) {
						pApp->encParam[cnt-1].enable_l2Cache = FALSE;
					} else {
						g_print ("Warning!! enable_l2Cache value is wrong taking default\n");
					}
				} else {
					g_print ("Warning!! enable_l2Cache value is wrong taking default\n");
				}
			} else if ((ptr = strstr (pApp->line, GDR_MODE)) != NULL) {
				ptr = get_value (ptr, GDR_MODE);
				if (ptr) {
					if (!strncasecmp (ptr, HORIZONTAL_GDR_MODE, strlen (HORIZONTAL_GDR_MODE))) {
						pApp->encParam[cnt-1].gdr_mode = GDR_MODE_HORIZONTAL;
					} else if (!strncasecmp (ptr, VERTICAL_GDR_MODE, strlen (VERTICAL_GDR_MODE))) {
						pApp->encParam[cnt-1].gdr_mode = GDR_MODE_VERTICAL;
					} else if (!strncasecmp (ptr, DISABLED_GDR_MODE, strlen (DISABLED_GDR_MODE))) {
						pApp->encParam[cnt-1].gdr_mode = GDR_MODE_DISABLED;
					} else {
						g_print ("Warning!! gdr mode is wrong taking default\n");
					}
				} else {
					g_print ("Warning!! gdr mode is wrong taking default\n");
				}
		   } else if ((ptr = strstr (pApp->line, ENTROPY_MODE)) != NULL) {
				ptr = get_value (ptr, ENTROPY_MODE);
				if (ptr) {
					if (!strncasecmp (ptr, CAVLC_ENTROPY_MODE, strlen (CAVLC_ENTROPY_MODE))) {
						pApp->encParam[cnt-1].entropy_mode = ENTROPY_MODE_CAVLC;
					} else if (!strncasecmp (ptr, CABAC_ENTROPY_MODE, strlen (CABAC_ENTROPY_MODE))) {
						pApp->encParam[cnt-1].entropy_mode = ENTROPY_MODE_CABAC;
					} else if (!strncasecmp (ptr, DEFAULT_ENTROPY_MODE, strlen (DEFAULT_ENTROPY_MODE))) {
						pApp->encParam[cnt-1].entropy_mode = ENTROPY_MODE_DEFAULT;
					} else {
						g_print ("Warning!! entropy mode is wrong taking default\n");
					}
				} else {
					g_print ("Warning!! entropy mode is wrong taking default\n");
				}
		   } else if ((ptr = strstr (pApp->line, MAX_PICTURE_SIZE)) != NULL) {
			   ptr = get_value (ptr, MAX_PICTURE_SIZE);
			   if (ptr) {
				   if (!strncasecmp (ptr, TRUE_VAL, strlen (TRUE_VAL))) {
					   pApp->encParam[cnt-1].max_picture_size = TRUE;
				   } else if (!strncasecmp (ptr, FALSE_VAL, strlen (FALSE_VAL))) {
					   pApp->encParam[cnt-1].max_picture_size = FALSE;
				   } else {
					   g_print ("Warning!! max_picture_size value is wrong taking default\n");
				   }
			   } else {
				   g_print ("Warning!! max_picture_size value is wrong taking default\n");
			   }
		   }
		}
    }
}

/**
 * \internal_start
 * \brief Get record configuration for the required channel
 *
 *
 * \sdd_start
 * <B>CSU Requirements and Constraints:</B>\n
 * \id_start get_record_config \id_end
 * \csu_start  \csu_end\n
 * \req_start \req_end\n
 *
 * <B>Algorithm & logic flow:</B>\n
 * - read all lines until reach the exit criteria:
 * 	-	update channel configuration
 * \sdd_end
 *
 * <B>Error handling & limitations:</B>\n
 *
 * \param guint channel - the required channel
 *
 * \return NONE.
 * \internal_end
 */
void
get_record_config (guint channel) {
    gchar *ptr = NULL;
    guint tmp, cnt = MIN_NUM_SOURCES;

    if((channel >= 0) && (channel < NUM_GST_CHANNELS)) {
		App* pApp = &app_data[channel];

		while (!meet_exit_criterion (channel)) {
			ptr = fgets (pApp->line, sizeof (pApp->line), pApp->file);
			if ((ptr = strstr (pApp->line, RECORD_NUM)) != NULL) {
				ptr = get_value (ptr, RECORD_NUM);
				if (ptr) {
					tmp = atoi (ptr);
					if (tmp >= MIN_NUM_SOURCES && tmp <= MAX_NUM_SOURCES) {
						cnt = tmp;
					} else {
						g_print ("Warning!! Record Num value is incorrect taking default\n");
					}
				}
			} else if ((ptr = strstr (pApp->line, OUT_FILE_NAME)) != NULL) {
				ptr = get_value (ptr, OUT_FILE_NAME);
				if (ptr) {
					pApp->opParam[cnt-1].file_out = g_strdup (ptr);
				} else {
					g_print ("Warning!! File out path is null\n");
				}
			} else if ((ptr = strstr (pApp->line, DURATION)) != NULL) {
				ptr = get_value (ptr, DURATION);
				if (ptr) {
					tmp = atoi (ptr);
					if (tmp >= MIN_RECORD_DUR && tmp <= MAX_RECORD_DUR) {
						pApp->opParam[cnt-1].duration = tmp;
					} else {
						g_print ("Warning!! Record duration is incorrect taking default\n");
					}
				} else {
					g_print ("Warning!! Record duration is incorrect taking default\n");
				}
			}
		}
    }
}

/**
 * \internal_start
 * \brief Get streaming configuration for the required channel
 *
 *
 * \sdd_start
 * <B>CSU Requirements and Constraints:</B>\n
 * \id_start get_stream_config \id_end
 * \csu_start  \csu_end\n
 * \req_start \req_end\n
 *
 * <B>Algorithm & logic flow:</B>\n
 * - read all lines until reach the exit criteria:
 * 	-	update channel configuration
 * \sdd_end
 *
 * <B>Error handling & limitations:</B>\n
 *
 * \param guint channel - the required channel
 *
 * \return NONE.
 * \internal_end
 */
void
get_stream_config (guint channel) {
    gchar *ptr = NULL;
    guint tmp, cnt = MIN_NUM_SOURCES;

    if((channel >= 0) && (channel < NUM_GST_CHANNELS)) {
		App* pApp = &app_data[channel];

		while (!meet_exit_criterion (channel)) {
			ptr = fgets (pApp->line, sizeof (pApp->line), pApp->file);
			if ((ptr = strstr (pApp->line, STREAMING_NUM)) != NULL) {
				ptr = get_value (ptr, STREAMING_NUM);
				tmp = atoi (ptr);
				if (tmp >= MIN_NUM_SOURCES && tmp <= MAX_NUM_SOURCES) {
					cnt = tmp;
				} else {
					g_print ("Streaming Num value is incorrect taking default\n");
				}
			} else if ((ptr = strstr (pApp->line, HOST_IP)) != NULL) {
				ptr = get_value (ptr, HOST_IP);
				if (ptr) {
					pApp->opParam[cnt-1].host_ip = g_strdup (ptr);
				} else {
					g_print ("Warning!! host_ip is null\n");
				}
			} else if ((ptr = strstr (pApp->line, PORT)) != NULL) {
				ptr = get_value (ptr, PORT);
				if (ptr) {
					pApp->opParam[cnt-1].port_num = atoi (ptr);
				} else {
					g_print ("Warning!! port_num is incorrect taking default\n");
				}
			}
		}
    }
}

/**
 * \internal_start
 * \brief Get audio configuration for the required channel
 *
 *
 * \sdd_start
 * <B>CSU Requirements and Constraints:</B>\n
 * \id_start get_audio_config \id_end
 * \csu_start  \csu_end\n
 * \req_start \req_end\n
 *
 * <B>Algorithm & logic flow:</B>\n
 * - read all lines until reach the exit criteria:
 * 	-	update channel configuration
 * \sdd_end
 *
 * <B>Error handling & limitations:</B>\n
 *
 * \param guint channel - the required channel
 *
 * \return NONE.
 * \internal_end
 */
void
get_audio_config (guint channel) {
    gchar *ptr = NULL;
    guint tmp =0;
    guint cnt = MIN_NUM_SOURCES;
    gdouble vol;

    if((channel >= 0) && (channel < NUM_GST_CHANNELS)) {
		App* pApp = &app_data[channel];

		while (!meet_exit_criterion (channel)) {
			ptr = fgets (pApp->line, sizeof (pApp->line), pApp->file);
			if ((ptr = strstr (pApp->line, AUDIO_NUM)) != NULL) {
				ptr = get_value (ptr, AUDIO_NUM);
				tmp = atoi (ptr);
				if (tmp >= MIN_NUM_SOURCES && tmp <= MAX_NUM_SOURCES) {
					cnt = tmp;
				} else {
					g_print ("Audio Num value is incorrect taking default\n");
				}
			} else if ((ptr = strstr (pApp->line, AUDIO_FORMAT)) != NULL) {
				ptr = get_value (ptr, AUDIO_FORMAT);
				if (ptr) {
					pApp->audParam[cnt-1].format = g_strdup (ptr);
				} else {
					g_print ("Warning!! audio format is null taking default\n");
				}
			} else if ((ptr = strstr (pApp->line, AUDIO_ENABLE)) != NULL) {
				ptr = get_value (ptr, AUDIO_ENABLE);
				if (ptr) {
					if (!strncasecmp (ptr, TRUE_VAL, strlen (TRUE_VAL))) {
						pApp->audParam[cnt-1].enable_audio = TRUE;
					} else if (!strncasecmp (ptr, FALSE_VAL, strlen (FALSE_VAL))) {
						pApp->audParam[cnt-1].enable_audio = FALSE;
					} else {
						g_print ("Warning!! enable_audio value is wrong taking default\n");
					}
				} else {
					g_print ("Warning!! enable_audio value is wrong taking default\n");
				}
			} else if ((ptr = strstr (pApp->line, SAMPLING_RATE)) != NULL) {
				ptr = get_value (ptr, SAMPLING_RATE);
				tmp = atoi (ptr);
				if (tmp) {
					pApp->audParam[cnt-1].sampling_rate = tmp;
				} else {
					g_print ("Warning!! sampling rate is incorrect taking default\n");
				}
			} else if ((ptr = strstr (pApp->line, NUM_CHANNEL)) != NULL) {
				ptr = get_value (ptr, NUM_CHANNEL);
				tmp = atoi (ptr);
				if (tmp) {
					pApp->audParam[cnt-1].channel = tmp;
				} else {
					g_print ("Warning!! channel is incorrect taking default\n");
				}
			} else if ((ptr = strstr (pApp->line, SOURCE)) != NULL) {
				ptr = get_value (ptr, SOURCE);
				if (ptr) {
					if (!strncasecmp (ptr, HDMI_INPUT, strlen (HDMI_INPUT))) {
						pApp->audParam[cnt-1].audio_in = AUDIO_HDMI_IN;
					} else if (!strncasecmp (ptr, SDI_INPUT, strlen (SDI_INPUT))) {
						pApp->audParam[cnt-1].audio_in = AUDIO_SDI_IN;
					} else if (!strncasecmp (ptr, I2S_INPUT, strlen (I2S_INPUT))) {
						pApp->audParam[cnt-1].audio_in = AUDIO_I2S_IN;
					} else {
						g_print ("Warning!! audio_in value is wrong taking default\n");
					}
				} else {
					g_print ("Warning!! audio_in value is wrong taking default\n");
				}
			} else if ((ptr = strstr (pApp->line, RENDERER)) != NULL) {
				ptr = get_value (ptr, RENDERER);
				if (ptr) {
					if (!strncasecmp (ptr, HDMI_OUT, strlen (HDMI_OUT))) {
						pApp->audParam[cnt-1].audio_out = AUDIO_HDMI_OUT;
					} else if (!strncasecmp (ptr, SDI_OUT, strlen (SDI_OUT))) {
						pApp->audParam[cnt-1].audio_out = AUDIO_SDI_OUT;
					} else if (!strncasecmp (ptr, I2S_OUT, strlen (I2S_OUT))) {
						pApp->audParam[cnt-1].audio_out = AUDIO_I2S_OUT;
					} else if (!strncasecmp (ptr, DP_OUT, strlen (DP_OUT))) {
						pApp->audParam[cnt-1].audio_out = AUDIO_DP_OUT;
					} else {
						g_print ("Warning!! audio_out value is wrong taking default\n");
					}
				} else {
					g_print ("Warning!! audio_out value is wrong taking default\n");
				}
			} else if ((ptr = strstr (pApp->line, VOLUME)) != NULL) {
				ptr = get_value (ptr, VOLUME);
				vol = atof (ptr);
				pApp->audParam[cnt-1].volume = vol;
			}
		}
    }
}

/**
 * \internal_start
 * \brief Get trace configuration for the required channel
 *
 *
 * \sdd_start
 * <B>CSU Requirements and Constraints:</B>\n
 * \id_start get_trace_config \id_end
 * \csu_start  \csu_end\n
 * \req_start \req_end\n
 *
 * <B>Algorithm & logic flow:</B>\n
 * - read all lines until reach the exit criteria:
 * 	-	update channel configuration
 * \sdd_end
 *
 * <B>Error handling & limitations:</B>\n
 *
 * \param guint channel - the required channel
 *
 * \return NONE.
 * \internal_end
 */
void
get_trace_config (guint channel) {
    gchar *ptr = NULL;

    if((channel >= 0) && (channel < NUM_GST_CHANNELS)) {
		App* pApp = &app_data[channel];

		while (!meet_exit_criterion (channel)) {
			ptr = fgets (pApp->line, sizeof (pApp->line), pApp->file);
			if ((ptr = strstr (pApp->line, FPS_INFO)) != NULL) {
				ptr = get_value (ptr, FPS_INFO);
				if (ptr) {
					if (!strncasecmp (ptr, TRUE_VAL, strlen (TRUE_VAL))) {
						pApp->fps_info = TRUE;
					} else if (!strncasecmp (ptr, FALSE_VAL, strlen (FALSE_VAL))) {
						pApp->fps_info = FALSE;
					} else {
						g_print ("Warning!! fps_info value is wrong taking default\n");
					}
				} else {
					g_print ("Warning!! fps_info value is wrong taking default\n");
				}
			} else if ((ptr = strstr (pApp->line, APM_INFO)) != NULL) {
				ptr = get_value (ptr, APM_INFO);
				if (ptr) {
					if (!strncasecmp (ptr, TRUE_VAL, strlen (TRUE_VAL))) {
						pApp->apm_info = TRUE;
					} else if (!strncasecmp (ptr, FALSE_VAL, strlen (FALSE_VAL))) {
						pApp->apm_info = FALSE;
					} else {
						g_print ("Warning!! apm_info value is wrong taking default\n");
					}
				} else {
					g_print ("Warning!! apm_info value is wrong taking default\n");
				}
			} else if ((ptr = strstr (pApp->line, LOOP_PLAYBACK)) != NULL) {
				ptr = get_value (ptr, LOOP_PLAYBACK);
				if (ptr) {
					if (!strncasecmp (ptr, TRUE_VAL, strlen (TRUE_VAL))) {
						pApp->loop_playback = TRUE;
					} else if (!strncasecmp (ptr, FALSE_VAL, strlen (FALSE_VAL))) {
						pApp->loop_playback = FALSE;
					} else {
						g_print ("Warning!! loop playback value is wrong taking default\n");
					}
				} else {
					g_print ("Warning!! loop playback value is wrong taking default\n");
				}
			} else if ((ptr = strstr (pApp->line, LOOP_INTERVAL)) != NULL) {
				ptr = get_value (ptr, LOOP_INTERVAL);
				if (ptr) {
					pApp->loop_interval = atoi (ptr);
				} else {
					g_print ("Warning!! loop interval in second is incorrect taking default\n");
				}
			} else if ((ptr = strstr (pApp->line, PIPELINE_INFO)) != NULL) {
				ptr = get_value (ptr, PIPELINE_INFO);
				if (ptr) {
					if (!strncasecmp (ptr, TRUE_VAL, strlen (TRUE_VAL))) {
						pApp->pipeline_info = TRUE;
					} else if (!strncasecmp (ptr, FALSE_VAL, strlen (FALSE_VAL))) {
						pApp->pipeline_info = FALSE;
					} else {
						g_print ("Warning!! pipeline_info value is wrong taking default\n");
					}
				} else {
					g_print ("Warning!! pipeline_info value is wrong taking default\n");
				}
			}
		}
    }
}


/**
 * \internal_start
 * \brief Parse configuration file and set the configuration accordingly
 *
 *
 * \sdd_start
 * <B>CSU Requirements and Constraints:</B>\n
 * \id_start parse_config_file \id_end
 * \csu_start  \csu_end\n
 * \req_start \req_end\n
 *
 * <B>Algorithm & logic flow:</B>\n
 * - read all lines:
 * 	-	update the relevant configuration: encoder / input / etc.
 * \sdd_end
 *
 * <B>Error handling & limitations:</B>\n
 *
 * \param guint channel - the required channel
 * \param gchar *path - path to configuration file
 *
 * \return gint - 0 on success, -1 on failure
 * \internal_end
 */
gint
parse_config_file (guint channel, gchar *path) {
    gchar *ptr = NULL;

    if((channel >= 0) && (channel < NUM_GST_CHANNELS)) {
		App* pApp = &app_data[channel];

		pApp->file = fopen (path, "r");
		if (!pApp->file) {
			g_print ("Error!! Input file can't be opened\n");
			return -1;
		}
		while (fgets (pApp->line, sizeof (pApp->line), pApp->file)) {
			if ((ptr = strstr (pApp->line, CMN_CONFIG)) != NULL) {
				get_cmn_config (channel);
			} else if ((ptr = strstr (pApp->line, INPUT_CONFIG)) != NULL) {
				get_input_config (channel);
			} else if ((ptr = strstr (pApp->line, ENCODER_CONFIG)) != NULL) {
				get_encoder_config (channel);
			} else if ((ptr = strstr (pApp->line, RECORD_CONFIG)) != NULL) {
				get_record_config (channel);
			} else if ((ptr = strstr (pApp->line, STREAMING_CONFIG)) != NULL) {
				get_stream_config (channel);
			} else if ((ptr = strstr (pApp->line, AUDIO_CONFIG)) != NULL) {
				get_audio_config (channel);
			} else if ((ptr = strstr (pApp->line, TRACE_CONFIG)) != NULL) {
				get_trace_config (channel);
			}
		}
		if ((RECORD == pApp->cmnParam.sink_type || STREAM ==  pApp->cmnParam.sink_type) && pApp->fps_info) {
		  g_print ("Warning!! fps info should be disabled for Record/streaming option\n");
		  pApp->fps_info = FALSE;
		}
		fclose (pApp->file);
		pApp->file = NULL;
    }
    return 0;
}

/**
 * \internal_start
 * \brief Quit the pipeline
 *
 *
 * \sdd_start
 * <B>CSU Requirements and Constraints:</B>\n
 * \id_start quit_pipeline \id_end
 * \csu_start  \csu_end\n
 * \req_start \req_end\n
 *
 * <B>Algorithm & logic flow:</B>\n
 * - stop the pipeline
 * \sdd_end
 *
 * <B>Error handling & limitations:</B>\n
 *
 * \param guint channel - the required channel
 *
 * \return gint - 0 on success, -1 on failure
 * \internal_end
 */
gint
quit_pipeline (guint channel) {
	gint ret = 0;
	if((channel >= 0) && (channel < NUM_GST_CHANNELS)) {
		ret = vgst_stop_pipeline(channel);

		if(VGST_SUCCESS == ret) {
			g_print("Pipeline stopped successfully \n");
		}
		else {
			g_print("ERROR: vgst_stop_pipeline() failed. \"%s\"\n", vgst_error_to_string(ret, 0,channel));
		}
	}
	else {
		ret = -1;
	}

	return ret;
}

/**
 * \internal_start
 * \brief Free all resources for the required channel
 *
 *
 * \sdd_start
 * <B>CSU Requirements and Constraints:</B>\n
 * \id_start free_resources \id_end
 * \csu_start  \csu_end\n
 * \req_start \req_end\n
 *
 * <B>Algorithm & logic flow:</B>\n
 * - free the resources: ip, host ip, format, etc.
 * \sdd_end
 *
 * <B>Error handling & limitations:</B>\n
 *
 * \param guint channel - the required channel
 *
 * \return NONE.
 * \internal_end
 */
void
free_resources (guint channel) {
    guint i =0, ret = 0;

    if((channel >= 0) && (channel < NUM_GST_CHANNELS)) {
		App* pApp = &app_data[channel];

		for (i =0; i< MAX_NUM_SOURCES; i++) {
			if (pApp->ipParam[i].uri) {
				g_free (pApp->ipParam[i].uri);
			}
			if (pApp->opParam[i].file_out) {
				g_free (pApp->opParam[i].file_out);
			}
			if (pApp->opParam[i].host_ip) {
				g_free (pApp->opParam[i].host_ip);
			}
			if (pApp->audParam[i].format) {
				g_free (pApp->audParam[i].format);
			}
			if (pApp->audParam[i].enable_audio) {
				if(pApp->audParam[i].source_id != NULL) {
					g_free(pApp->audParam[i].source_id);
				}
				if (pApp->audParam[i].sink_id != NULL) {
					g_free(pApp->audParam[i].sink_id);
				}
			}
		}
		if (pApp->cmnParam.bus_id) {
			g_free (pApp->cmnParam.bus_id);
		}
		if (pApp->file) {
			fclose (pApp->file);
			pApp->file = NULL;
		}
		if (pApp->apm_info) {
			if (perf_monitor_deinit() != EXIT_SUCCESS) {
				g_print ("APM de-initialization failed\n");
			}
		}
		ret = vgst_uninit();
		if (ret !=  VGST_SUCCESS) {
			g_print("ERROR: vgst_uninit() failed error code \"%s\"\n", vgst_error_to_string(ret, 0,channel));
		}
    }
}

/**
 * \internal_start
 * \brief Set default parameters for the required channel
 *
 *
 * \sdd_start
 * <B>CSU Requirements and Constraints:</B>\n
 * \id_start reset_all_params \id_end
 * \csu_start  \csu_end\n
 * \req_start \req_end\n
 *
 * <B>Algorithm & logic flow:</B>\n
 * - set default parameters, such as: bit rate, encoder type, etc.
 * \sdd_end
 *
 * <B>Error handling & limitations:</B>\n
 *
 * \param guint channel - the required channel
 *
 * \return NONE.
 * \internal_end
 */
void
reset_all_params (guint channel) {
	if((channel >= 0) && (channel < NUM_GST_CHANNELS)) {
	    guint i =0;
		App* pApp = &app_data[channel];

	    pApp->channel = channel;
	    pApp->cmnParam.driver_type = HDMI_Tx;
	    pApp->cmnParam.frame_rate = DEFAULT_DISPLAY_RATE;
	    pApp->cmnParam.num_src = MIN_NUM_SOURCES;
	    pApp->cmnParam.sink_type = DISPLAY;
	    pApp->fps_info = TRUE;
	    pApp->apm_info = TRUE;
	    pApp->pipeline_info = TRUE;
	    pApp->loop_playback = FALSE;
	    pApp->loop_interval = 5;
	    pApp->position = -1;

	    for (i =0; i< MAX_NUM_SOURCES; i++) {
	    	pApp->encParam[i].b_frame = DEFAULT_B_FRAME;
	    	pApp->encParam[i].bitrate = DEFAULT_BITRATE;
	    	pApp->encParam[i].enable_l2Cache = TRUE;
	    	pApp->encParam[i].enc_type = HEVC;
	    	pApp->encParam[i].gop_len = DEFAULT_GOP_LEN;
	        if (pApp->encParam[i].enc_type == HEVC)
	        	pApp->encParam[i].profile = MAIN_PROFILE;
	        else
	        	pApp->encParam[i].profile = HIGH_PROFILE;
	        pApp->encParam[i].qp_mode = AUTO;
	        pApp->encParam[i].rc_mode = CBR;
	        pApp->encParam[i].slice = DEFAULT_NUM_SLICE;
	        pApp->encParam[i].gop_mode = BASIC;
	        pApp->encParam[i].filler_data = TRUE;
	        pApp->encParam[i].low_bandwidth = FALSE;
	        pApp->encParam[i].hlg_sdr_compatible = FALSE;
	        pApp->encParam[i].latency_mode = NORMAL_LATENCY;
	        pApp->encParam[i].gdr_mode = GDR_MODE_DISABLED;
	        if (pApp->encParam[i].enc_type == HEVC)
	            pApp->encParam[i].entropy_mode = ENTROPY_MODE_CABAC;
	        else
	            pApp->encParam[i].entropy_mode = ENTROPY_MODE_CAVLC;
	        pApp->encParam[i].max_picture_size = FALSE;

	        /* input param initialization */
	        pApp->ipParam[i].device_type = TPG_1;//TPG_2
	        pApp->ipParam[i].format = NV12;
	        pApp->ipParam[i].raw = FALSE;
	        pApp->ipParam[i].src_type = LIVE_SRC;
	        pApp->ipParam[i].width = MAX_WIDTH;
	        pApp->ipParam[i].height = MAX_HEIGHT;
	        pApp->ipParam[i].relative_qp = DEFAULT_RELATIVE_QP;
	        pApp->ipParam[i].accelerator = FALSE;
	        pApp->ipParam[i].enable_scd = FALSE;
	        pApp->ipParam[i].enable_roi = FALSE;
	        pApp->ipParam[i].enable_llp2 = FALSE;

	        /* output param initialization */
	        pApp->opParam[i].duration = MIN_RECORD_DUR;
	        pApp->opParam[i].port_num = DEFAULT_PORT_NUM;

	        pApp->update_bitrate[i] = TRUE;
	        pApp->audParam[i].format = g_strdup("S24_32LE");
	        pApp->audParam[i].enable_audio = FALSE;
	        pApp->audParam[i].sampling_rate = 48000;
	        pApp->audParam[i].channel = 2;
	        pApp->audParam[i].volume = 2.0;
	        pApp->audParam[i].audio_in = AUDIO_HDMI_IN;
	        pApp->audParam[i].audio_out = AUDIO_HDMI_OUT;

	    }
	}
}

/**
 * \internal_start
 * \brief Print pipeline information for the required channel
 *
 *
 * \sdd_start
 * <B>CSU Requirements and Constraints:</B>\n
 * \id_start print_pipeline_cmd \id_end
 * \csu_start  \csu_end\n
 * \req_start \req_end\n
 *
 * <B>Algorithm & logic flow:</B>\n
 * - Print pipeline information: ip, host ip, format, etc.
 * \sdd_end
 *
 * <B>Error handling & limitations:</B>\n
 *
 * \param guint channel - the required channel
 *
 * \return NONE.
 * \internal_end
 */
void
print_pipeline_cmd (guint channel) {
    guint cnt =0;
    gchar *sink;
    if((channel >= 0) && (channel < NUM_GST_CHANNELS)) {
		App* pApp = &app_data[channel];

		sink = pApp->cmnParam.sink_type == RECORD ? RECORD_FILE_OUT: pApp->cmnParam.sink_type == STREAM ? STREAM_OUT:DISPLAY_OUT;
		g_print ("/*************************Pipeline Information Start*************************/\n");
		g_print ("Pipeline Info : %s\n", pApp->pipeline_info == TRUE ? "On" : "Off");
		g_print ("Fps Info : %s\n", pApp->fps_info == TRUE ? "On" : "Off");
		g_print ("APM Info : %s\n", pApp->apm_info == TRUE ? "On" : "Off");
		g_print ("Output goes on : %s\n", pApp->cmnParam.driver_type == DP ? DP_OUT: pApp->cmnParam.driver_type == HDMI_Tx ? HDMI_OUT : SDI_OUT);
		g_print ("Frame rate : %d\n", pApp->cmnParam.frame_rate);
		g_print ("Number of Source is : %d\n", pApp->cmnParam.num_src);
		g_print ("Use case is to : %s\n", sink);
		for (cnt =0; cnt< pApp->cmnParam.num_src; cnt++) {
			if (pApp->audParam[cnt].enable_audio) {
				g_print ("Audio format is : %s\n", pApp->audParam[cnt].format);
				g_print ("Audio sampling rate is : %d\n", pApp->audParam[cnt].sampling_rate);
				g_print ("Channel is : %d\n", pApp->audParam[cnt].channel);
				g_print ("Volume level is : %f\n", pApp->audParam[cnt].volume);
				g_print ("Audio Source is : %s\n", pApp->audParam[cnt].audio_in == AUDIO_HDMI_IN ? HDMI_INPUT : pApp->audParam[cnt].audio_in == AUDIO_SDI_IN ? SDI_INPUT : I2S_INPUT);
				g_print ("Audio Renderer is : %s\n", pApp->audParam[cnt].audio_out == AUDIO_HDMI_OUT ? HDMI_OUT : pApp->audParam[cnt].audio_out == AUDIO_SDI_OUT ? SDI_OUT : \
						  pApp->audParam[cnt].audio_out == AUDIO_I2S_OUT ? I2S_OUT : DP_OUT);
			}
			if (!(pApp->ipParam[cnt].src_type == FILE_SRC || pApp->ipParam[cnt].src_type == STREAMING_SRC || pApp->ipParam[cnt].raw)) {
				g_print ("B Frames : %d\n", pApp->encParam[cnt].b_frame);
				g_print ("Bitrate : %d\n", pApp->encParam[cnt].bitrate);
				g_print ("Enable L2Cache : %s\n", pApp->encParam[cnt].enable_l2Cache == TRUE ? "True" : "False");
				g_print ("Enc Name : %s\n", pApp->encParam[cnt].enc_type == AVC ? "AVC":"HEVC");
				g_print ("Gop Len : %d\n", pApp->encParam[cnt].gop_len);
				g_print ("Profile : %s\n", pApp->encParam[cnt].profile == BASELINE_PROFILE? BASE_PROF : \
						  pApp->encParam[cnt].profile == MAIN_PROFILE ? MAIN_PROF : HIGH_PROF);
				g_print ("Qp Mode : %s\n", pApp->encParam[cnt].qp_mode == AUTO ? AUTO_QP : pApp->encParam[cnt].qp_mode == UNIFORM ? UNIFORM_QP : ROI_QP);
				g_print ("Rc Mode : %s\n", pApp->encParam[cnt].rc_mode == CBR? CBR_RC:pApp->encParam[cnt].rc_mode == VBR? VBR_RC : LOWLATENCY_RC);
				g_print ("Num Slice : %d\n", pApp->encParam[cnt].slice);
				g_print ("GoP Mode : %s\n", pApp->encParam[cnt].gop_mode == BASIC ? BASIC_GOP : pApp->encParam[cnt].gop_mode == LOW_DELAY_P ? LOW_DELAY_P_GOP : \
											LOW_DELAY_B_GOP);
				g_print ("Filler Data : %s\n", pApp->encParam[cnt].filler_data == TRUE ? "True" : "False");
				g_print ("Low Bandwidth : %s\n", pApp->encParam[cnt].low_bandwidth == TRUE ? "True" : "False");
				g_print ("HLG_SDR_Compatible : %s\n", pApp->encParam[cnt].hlg_sdr_compatible == TRUE ? "True" : "False");
				g_print ("Latency Mode : %s\n", pApp->encParam[cnt].latency_mode == NORMAL_LATENCY ? NORMAL : SUB_FRAME);
				g_print ("GDR Mode : %s\n", pApp->encParam[cnt].gdr_mode == GDR_MODE_VERTICAL ? VERTICAL_GDR_MODE : pApp->encParam[cnt].gdr_mode == GDR_MODE_HORIZONTAL ? HORIZONTAL_GDR_MODE : DISABLED_GDR_MODE);
				g_print ("Entropy Mode : %s\n", pApp->encParam[cnt].entropy_mode == ENTROPY_MODE_CAVLC ? CAVLC_ENTROPY_MODE : pApp->encParam[cnt].entropy_mode == ENTROPY_MODE_CABAC ? CABAC_ENTROPY_MODE : DEFAULT_ENTROPY_MODE);
				g_print ("Max Picture Size : %s\n", pApp->encParam[cnt].max_picture_size == TRUE ? "True" : "False");
			}
			g_print ("Device Type : %s\n", pApp->ipParam[cnt].device_type == TPG_1 ? \
					  TPG_1_INPUT : pApp->ipParam[cnt].device_type == TPG_2 ? TPG_2_INPUT : pApp->ipParam[cnt].device_type == HDMI_1 ? \
					  HDMI_INPUT: pApp->ipParam[cnt].device_type == CSI ? MIPI_INPUT : pApp->ipParam[cnt].device_type == HDMI_2 ? \
					  HDMI_2_INPUT : pApp->ipParam[cnt].device_type == HDMI_3 ? HDMI_3_INPUT : pApp->ipParam[cnt].device_type == HDMI_4 ? \
					  HDMI_4_INPUT : pApp->ipParam[cnt].device_type == HDMI_5 ? HDMI_5_INPUT : pApp->ipParam[cnt].device_type == HDMI_6 ? \
					  HDMI_6_INPUT : pApp->ipParam[cnt].device_type == HDMI_7 ? HDMI_7_INPUT : pApp->ipParam[cnt].device_type == CSI_2 ? \
					  CSI_2_INPUT : pApp->ipParam[cnt].device_type == CSI_3 ? CSI_3_INPUT : pApp->ipParam[cnt].device_type == CSI_4 ? \
					  CSI_4_INPUT : SDI_INPUT);
			g_print ("Format : %s\n", pApp->ipParam[cnt].format == NV12 ? \
					  NV12_FORMAT : pApp->ipParam[cnt].format == NV16 ? \
					  NV16_FORMAT : pApp->ipParam[cnt].format == XV15 ? XV15_FORMAT : XV20_FORMAT);
			g_print ("Width : %d\n", pApp->ipParam[cnt].width);
			g_print ("Height : %d\n", pApp->ipParam[cnt].height);
			g_print ("Relative QP : %d\n", pApp->ipParam[cnt].relative_qp);
			g_print ("Raw : %s\n", pApp->ipParam[cnt].raw == TRUE ? "True" : "False");
			g_print ("Accelerator flag : %s\n", pApp->ipParam[cnt].accelerator == TRUE ? "True" : "False");
			g_print ("Enable_scd flag : %s\n", pApp->ipParam[cnt].enable_scd == TRUE ? "True" : "False");
			g_print ("Enable_roi flag : %s\n", pApp->ipParam[cnt].enable_roi == TRUE ? "True" : "False");
			g_print ("Enable_llp2 flag : %s\n", pApp->ipParam[cnt].enable_llp2 == TRUE ? "True" : "False");
			g_print ("Src Type : %s\n", pApp->ipParam[cnt].src_type ==LIVE_SRC ? "Live Src" : pApp->ipParam[cnt].src_type ==FILE_SRC ? "File Src" : "Streaming Src");
			if (pApp->ipParam[cnt].uri)
				g_print ("URI : %s\n", pApp->ipParam[cnt].uri);
			if (!strncasecmp (sink, RECORD_FILE_OUT, strlen (RECORD_FILE_OUT))) {
				g_print ("Duration : %d\n", pApp->opParam[cnt].duration);
				g_print ("Recording file path : %s\n", pApp->opParam[cnt].file_out);
			} else if (!strncasecmp (sink, STREAM_OUT, strlen (STREAM_OUT))) {
				g_print ("Streaming at IP : %s\n", pApp->opParam[cnt].host_ip);
				g_print ("Streaming at Port : %d\n", pApp->opParam[cnt].port_num);
			}
		}
		g_print ("/*************************Pipeline Information End*************************/\n");
    }
}

/**
 * \internal_start
 * \brief Update database (CSC) parameters for the required channel
 *
 *
 * \sdd_start
 * <B>CSU Requirements and Constraints:</B>\n
 * \id_start vgst_app_update_database_param \id_end
 * \csu_start  \csu_end\n
 * \req_start \req_end\n
 *
 * <B>Algorithm & logic flow:</B>\n
 * - Update database (CSC) parameters: ip, host ip, format, etc.
 * \sdd_end
 *
 * <B>Error handling & limitations:</B>\n
 *
 * \param guint channel - the required channel
 *
 * \return NONE.
 * \internal_end
 */
void
vgst_app_update_database_param (guint channel) {
	if((channel >= 0) && (channel < NUM_GST_CHANNELS)) {
		App* pApp = &app_data[channel];
		int i = 0;
		DBS_tsEnc_Ch_Create_Params sEnc_Ch_Create_Params = {};

		sEnc_Ch_Create_Params.eDest_Addr_Type = DBS_CH_TS_ETH;
		struct sockaddr_in addr;
		addr.sin_addr.s_addr = inet_addr(pApp->opParam[i].host_ip);
		sEnc_Ch_Create_Params.sDest_Address.u32Address = ntohl(addr.sin_addr.s_addr);
		sEnc_Ch_Create_Params.sDest_Address.u32Port_Num = pApp->opParam[i].port_num;
		/*-----------------------------------------------------------------------------*/
		sEnc_Ch_Create_Params.u32Frame_Rate = (1000 * pApp->cmnParam.frame_rate);
		sEnc_Ch_Create_Params.u32Enc_Res_Height = pApp->ipParam[i].height;
		sEnc_Ch_Create_Params.u32Enc_Res_Width = pApp->ipParam[i].width;
		// sEnc_Ch_Create_Params.eScan_Type (interlaced progressive) is not supported by the FPGA
		sEnc_Ch_Create_Params.i32Intra_Frame_Interval = pApp->encParam[i].gop_len;
		sEnc_Ch_Create_Params.i32Inter_Frame_Interval = pApp->encParam[i].b_frame;
		sEnc_Ch_Create_Params.sDest_Address.u32Port_Num = pApp->opParam[i].port_num;
		sEnc_Ch_Create_Params.u32Bit_Rate_Value = pApp->encParam[i].bitrate;


		switch (pApp->encParam[i].profile) {
			case BASELINE_PROFILE:
				sEnc_Ch_Create_Params.eEnc_Codec_Profile = DBS_H264_BASELINE_PROFILE;
				break;
			case MAIN_PROFILE:
				sEnc_Ch_Create_Params.eEnc_Codec_Profile = DBS_H264_MAIN_PROFILE;
				break;
			case HIGH_PROFILE:
				sEnc_Ch_Create_Params.eEnc_Codec_Profile = DBS_H264_HIGH_PROFILE;
				break;

			default:
				break;
		}

		switch (pApp->encParam[i].enc_type) {
			case AVC:
				sEnc_Ch_Create_Params.eVideo_Codec = DBS_ENC_VIDEO_CODEC_H264;
				break;
			case HEVC:
				sEnc_Ch_Create_Params.eVideo_Codec = DBS_ENC_VIDEO_CODEC_H265;
				break;
			default:
				break;
		}

		switch (pApp->ipParam[i].device_type) {
			case TPG_1:
				sEnc_Ch_Create_Params.eVAM_Video_In = DBS_ARINC_SRC_SVID_0;
				break;
			case TPG_2:
				sEnc_Ch_Create_Params.eVAM_Video_In = DBS_ARINC_SRC_SVID_1;
				break;
			default:
				break;
		}

		switch (pApp->encParam[i].rc_mode) {
			case VBR:
				sEnc_Ch_Create_Params.eBit_Rate_Type = DBS_ENC_VBR;
				break;
			case CBR:
				sEnc_Ch_Create_Params.eBit_Rate_Type = DBS_ENC_CBR;
				break;
			default:
				break;
		}

		/* audio - currently unused/not supported. set as false */
		sEnc_Ch_Create_Params.u32Audio_Enabled = FALSE;
	//	sEnc_Ch_Create_Params.u32Audio_Enabled = pApp->audParam.enable_audio;
	//	sEnc_Ch_Create_Params.u32Audio_Bit_Rate = pApp->audParam.sampling_rate;

		/* meta data - not supported. set as false */
		sEnc_Ch_Create_Params.u32Meta_Data_Mode = (UInt32)MDATA_DISABLE_MODE;

		DBS_srvSet_All_Enc_Ch_Create_Params(channel, &sEnc_Ch_Create_Params);
	}

}

/**
 * \internal_start
 * \brief Handle signal ctrl+C - for stand alone case
 *
 *
 * \sdd_start
 * <B>CSU Requirements and Constraints:</B>\n
 * \id_start signal_handler \id_end
 * \csu_start  \csu_end\n
 * \req_start \req_end\n
 *
 * <B>Algorithm & logic flow:</B>\n
 * - stop encoding
 * \sdd_end
 *
 * <B>Error handling & limitations:</B>\n
 *
 * \param gint sig - the signal
 *
 * \return NONE.
 * \internal_end
 */
void
signal_handler (gint sig) {
	signal(sig, SIG_IGN);
	if((standalone_channal_number >= 0) && (standalone_channal_number < NUM_GST_CHANNELS)) {
		 g_print("\n"
				 "Ctrl-C was hit.\n"
				  "Quitting the app now\n");
		 /*app_data[channel].loop_playback = FALSE;
		 if (app_data[channel].loop && g_main_is_running (app_data[channel].loop)) {
			 g_print ("Quitting the loop \n");
			 g_main_loop_quit (app_data[channel].loop);
			 g_main_loop_unref (app_data[channel].loop);
		 }*/
		vgst_app_stop_encode(standalone_channal_number); // receive current channel, verify this parameter update correctly
	}
    return;
}

/**
 * \internal_start
 * \brief A function to handle timeout in case of loop playback
 *
 *
 * \sdd_start
 * <B>CSU Requirements and Constraints:</B>\n
 * \id_start loop_cb \id_end
 * \csu_start  \csu_end\n
 * \req_start \req_end\n
 *
 * <B>Algorithm & logic flow:</B>\n
 * - quit pipeline
 * - quit loop (free resources, etc.)
 * \sdd_end
 *
 * <B>Error handling & limitations:</B>\n
 *
 * \param gpointer arg - the required channel
 *
 * \return gboolean - false in case of failure, ending the loop otherwise
 * \internal_end
 */
gboolean
loop_cb (gpointer arg) {
	guint channel = *(guint*)arg;
	if((channel >= 0) && (channel < NUM_GST_CHANNELS)) {
		App* pApp = &app_data[channel];

		quit_pipeline (channel);
		if (pApp->loop_playback) {
			if (pApp->loop && g_main_loop_is_running (pApp->loop)) {
				g_print ("Quitting the loop \n");
				g_main_loop_quit (pApp->loop);
				g_main_loop_unref (pApp->loop);
			}
		}
	}
    return FALSE;
}

/**
 * \internal_start
 * \brief A function to handle events
 *
 *
 * \sdd_start
 * <B>CSU Requirements and Constraints:</B>\n
 * \id_start time_cb \id_end
 * \csu_start  \csu_end\n
 * \req_start \req_end\n
 *
 * <B>Algorithm & logic flow:</B>\n
 * - handle event according to the event type
 * \sdd_end
 *
 * <B>Error handling & limitations:</B>\n
 *
 * \param gpointer parg - pointer to the required channel
 *
 * \return gboolean - false in case of failure, TRUE otherwise
 * \internal_end
 */
gboolean
time_cb (gpointer parg) {
	guint channel = *(guint*)parg;
    gint arg = 0;
    guint ret = 0, cnt =0;

    if((channel >= 0) && (channel < NUM_GST_CHANNELS)) {
		App* pApp = &app_data[channel];
		guint num_src = pApp->cmnParam.num_src;
		guint bitrate =0;


		ret = vgst_poll_event(&arg, cnt, channel);
		if (EVENT_EOS == ret) {
			if (pApp->loop && g_main_loop_is_running (pApp->loop)) {
				g_print ("Quitting the loop \n");
				g_main_loop_quit (pApp->loop);
				g_main_loop_unref (pApp->loop);
			}
			return FALSE;
		} else {
			for (cnt = 0; cnt < num_src; cnt++) {
				ret = vgst_poll_event(&arg, cnt,channel);
				if (EVENT_ERROR == ret) {
					g_print ("Error!! code \"%s\" to pipeline [%d]\n", vgst_error_to_string (arg, cnt,channel), cnt+1);
				} else {
					if (pApp->apm_info) {
						pid_t pid = getpid();
						double enc_bandw[] = {perf_monitor_get_rd_wr_cnt(E_APM0, E_READ_BYTE_CNT)* BYTE_TO_GBIT,
											  perf_monitor_get_rd_wr_cnt(E_APM0, E_WRITE_BYTE_CNT)* BYTE_TO_GBIT,
											  perf_monitor_get_rd_wr_cnt(E_APM1, E_READ_BYTE_CNT)* BYTE_TO_GBIT,
											  perf_monitor_get_rd_wr_cnt(E_APM1, E_WRITE_BYTE_CNT)* BYTE_TO_GBIT};
						double dec_bandw[] = {perf_monitor_get_rd_wr_cnt(E_APM2, E_READ_BYTE_CNT)* BYTE_TO_GBIT,
											  perf_monitor_get_rd_wr_cnt(E_APM2, E_WRITE_BYTE_CNT)* BYTE_TO_GBIT,
											  perf_monitor_get_rd_wr_cnt(E_APM3, E_READ_BYTE_CNT)* BYTE_TO_GBIT,
											  perf_monitor_get_rd_wr_cnt(E_APM3, E_WRITE_BYTE_CNT)* BYTE_TO_GBIT};
						double enc_bandw_total[] = {enc_bandw[0] + enc_bandw[1], enc_bandw[2] + enc_bandw[3]};
						double dec_bandw_total[] = {dec_bandw[0] + dec_bandw[1], dec_bandw[2] + dec_bandw[3]};
						if(enc_bandw_total[0] >= 0.01)
							g_print ("Pid[%d]: Encoder Memory Bandwidth (APM0 %2.2f: R0 %2.2f W0 %2.2f Gbps)\n", pid,
									enc_bandw_total[0], enc_bandw[0], enc_bandw[1]);
						if(enc_bandw_total[1] >= 0.01)
							g_print ("Pid[%d]: Encoder Memory Bandwidth (AMP1 %2.2f: R1 %2.2f W1 %2.2f Gbps)\n", pid,
									enc_bandw_total[1], enc_bandw[2], enc_bandw[3]);
						if(dec_bandw_total[0] >= 0.01)
							g_print ("Pid[%d]: Decoder Memory Bandwidth (AMP2 %2.2f: R2 %2.2f W2 %2.2f Gbps)\n", pid,
									dec_bandw_total[0], dec_bandw[0], dec_bandw[1]);
						if(dec_bandw_total[1] >= 0.01)
							g_print ("Pid[%d]: Decoder Memory Bandwidth (AMP2 %2.2f: R3 %2.2f W3 %2.2f Gbps)\n", pid,
									dec_bandw_total[1], dec_bandw[2], dec_bandw[3]);
					}
					if (pApp->fps_info) {
						vgst_get_fps (cnt, pApp->fps, channel);
						if (SPLIT_SCREEN == pApp->cmnParam.sink_type)
							g_print ("Split screen Pipeline [%d] Fps[%d]::[%d]\n", cnt+1, pApp->fps[0], pApp->fps[1]);
						else
							g_print ("Pipeline [%d] Fps[%d]\n", cnt+1, pApp->fps[0]);
					}
					if (RECORD == pApp->cmnParam.sink_type) {
						vgst_get_position (cnt, &pApp->position, channel);
					}
					if ((pApp->ipParam[cnt].src_type == FILE_SRC || pApp->ipParam[cnt].src_type == STREAMING_SRC) && pApp->update_bitrate[cnt]) {
						bitrate = vgst_get_bitrate (cnt, channel);
						if (bitrate) {
							if (round(BIT_TO_MBIT(bitrate)) <= 0.0){
								g_print ("pipeline [%d] file/streaming Bitrate : [%f]Kbps\n", cnt+1, round(BIT_TO_KBIT(bitrate)));
							} else {
								g_print ("pipeline [%d] file/streaming Bitrate : [%f]Mbps\n", cnt+1, round(BIT_TO_MBIT(bitrate)));
							}
							pApp->update_bitrate[cnt] = FALSE;
						}
					}
				}
			}
		}
    }
    return TRUE;
}

/**
 * \internal_start
 * \brief A function to initialize vgst app
 *
 *
 * \sdd_start
 * <B>CSU Requirements and Constraints:</B>\n
 * \id_start vgst_app_init \id_end
 * \csu_start  \csu_end\n
 * \req_start \req_end\n
 *
 * <B>Algorithm & logic flow:</B>\n
 * - check if it stand alone:
 * 	- if not, according to received params initialize the relevant channel with data fron CFG file
 * 	- if yes, initialize all available channels with default params
 * \sdd_end
 *
 * <B>Error handling & limitations:</B>\n
 *
 * \param gpointer parg - pointer to the required channel
 *
 * \return gint - channel number if succeed, other value otherwise
 * \internal_end
 */
gint
vgst_app_init (gboolean standalone, gint argc, gchar *argv[]) {
	static char config_file_path[256]={};
    gint ret = 0;
	gint channel = 0;
	App* pApp;

	if(!standalone)
	{
		if (argc < 3) {
			g_print ("Error!! Less arguments see the usage note :\n");
			usage ();
			return -1;
		}
		
		channel = atoi(argv[1]);
		if(channel < 0 || channel >= (sizeof(app_data)/sizeof(app_data[0]))) {
			g_print ("Error!! Wrong channel value %d :\n", channel);
			usage ();
			return -1;
		}
		
		reset_all_params (channel);

		snprintf(config_file_path, sizeof(config_file_path)-1, "%s/%s", argv[0], argv[2]);
		if (parse_config_file (channel, config_file_path) < 0) {
			g_print ("Error!! parsing CFG file '%s' failed, trying %s\n",
					config_file_path, argv[2]);
			if (parse_config_file (channel, argv[2]) < 0) {
				g_print ("Error!! parsing CFG file '%s' failed\n", argv[2]);
				return -1;
			}
		}

		/* update database params */
		vgst_app_update_database_param(channel);
	}
	else
	{
		for(channel=0; channel<NUM_GST_CHANNELS ; channel++)
		{
			reset_all_params (channel);
		}
	}

	for(channel=0; channel<NUM_GST_CHANNELS ; channel++)
	{
		if(!standalone)
		{
			channel = atoi(argv[1]);
		}
		pApp = &app_data[channel];

		ret = vgst_init ();
		if (ret !=  VGST_SUCCESS) {
			g_print("ERROR: vgst_init() failed error code %d :: \"%s\"\n", ret, vgst_error_to_string(ret, 0,channel));
			if (ret ==  VLIB_NO_MEDIA_SRC) {
				g_print("Get \"%s\" error in Tx only design, can be ignored!!!\n",  vgst_error_to_string(ret, 0,channel));
			} else {
				g_print("Error condition : CleanUp\n");
				return ret;
			}
		}

		if (pApp->apm_info) {
			//ODELYA - TODO: edit APM to support more than 1 channel
			if (perf_monitor_init() != EXIT_SUCCESS ) {
				g_print ("APM initialization failed. Disabling APM logs\n");
				pApp->apm_info = FALSE;
			}
		}

#if TRIAL_CODE_4_SET_FORMAT
		struct media_device *media = media_device_new("/dev/media0");
		if(media) {
			ret = media_device_enumerate(media);
			if(ret >= 0) {
				#define MEDIA_FMT				"\"%s\":%d [fmt:%s/%dx%d]"
				#define MEDIA_FMT_INT			"%d:%d [fmt:%s/%dx%d]"
				#define MEDIA_TPG_FMT_IN		"VYYUYY8"
				#define MEDIA_TPG_ENTITY_SUFFIX	".v_tpg"
				#define MEDIA_TPG_ENTITY_NUM	(5)
				#define MEDIA_TPG_PAD_NUM		(0)
				char fmt_str[100]={};
				sprintf( fmt_str, MEDIA_FMT, MEDIA_TPG_ENTITY_SUFFIX, MEDIA_TPG_PAD_NUM, MEDIA_TPG_FMT_IN, 3840, 2160);
				ret = v4l2_subdev_parse_setup_formats(media, fmt_str);
			}
			media_device_unref(media);
		}
#endif

		if (pApp->pipeline_info)
			print_pipeline_cmd (channel);

		g_timeout_add_seconds (DEFAULT_INFO_INTERVAL, (GSourceFunc)time_cb, (gpointer)&pApp->channel);

		if(!standalone)
		{
			break;
		}
	}

	return channel;
}

/**
 * \internal_start
 * \brief Free resources of the required channel
 *
 *
 * \sdd_start
 * <B>CSU Requirements and Constraints:</B>\n
 * \id_start vgst_app_deinit \id_end
 * \csu_start  \csu_end\n
 * \req_start \req_end\n
 *
 * <B>Algorithm & logic flow:</B>\n
 * - call @free_resources
 * \sdd_end
 *
 * <B>Error handling & limitations:</B>\n
 *
 * \param guint channel - the required channel
 *
 * \return NONE.
 * \internal_end
 */
void
vgst_app_deinit (guint channel) {
	if((channel >= 0) && (channel < NUM_GST_CHANNELS))
	{
		free_resources (channel);
	}
}


/**
 * \internal_start
 * \brief Start encode
 *
 *
 * \sdd_start
 * <B>CSU Requirements and Constraints:</B>\n
 * \id_start vgst_app_start_encode \id_end
 * \csu_start  \csu_end\n
 * \req_start \req_end\n
 *
 * <B>Algorithm & logic flow:</B>\n
 * - start the encoder
 * \sdd_end
 *
 * <B>Error handling & limitations:</B>\n
 *
 * \param guint channel - the required channel
 *
 * \return gint - 0 if succeed, other value if failed
 * \internal_end
 */
gint
vgst_app_start_encode(gint channel)
{
	gint ret = 0;
	if((channel >= 0) && (channel < NUM_GST_CHANNELS))
	{
		int count = 0;
		int i = 0;
		App* pApp = &app_data[channel];
		char adr[NUM_GST_CHANNELS][32];

		//vgst_app_init (TRUE, 0, NULL);

	    DBS_tsEnc_Ch_Create_Params sEnc_Ch_Create_Params = {};
	    DBS_srvGet_All_Enc_Ch_Create_Params(channel, &sEnc_Ch_Create_Params);

	    struct in_addr addr;
		/* get the binary form of the IP address and swap it */
		addr.s_addr = ntohl(sEnc_Ch_Create_Params.sDest_Address.u32Address);
		/* convert the binary form into dotted-decimal notation */
	    char *ipaddr = inet_ntoa(addr);
	    strcpy(adr[channel], ipaddr);
	    pApp->opParam[i].host_ip = adr[channel];
	    pApp->opParam[i].port_num = sEnc_Ch_Create_Params.sDest_Address.u32Port_Num;

	    /*-----------------------------------------------------------------------------*/
	    pApp->cmnParam.frame_rate=(sEnc_Ch_Create_Params.u32Frame_Rate+500)/1000;
	    pApp->ipParam[i].height = sEnc_Ch_Create_Params.u32Enc_Res_Height;
		pApp->ipParam[i].width = sEnc_Ch_Create_Params.u32Enc_Res_Width;
		pApp->encParam[i].gop_len = sEnc_Ch_Create_Params.i32Intra_Frame_Interval;
		pApp->encParam[i].b_frame = sEnc_Ch_Create_Params.i32Inter_Frame_Interval;

		// check min & max bitrate
		if(sEnc_Ch_Create_Params.u32Bit_Rate_Value < (LOW_BITRATE)) {
			pApp->encParam[i].bitrate = LOW_BITRATE;
		} else if (sEnc_Ch_Create_Params.u32Bit_Rate_Value > (HIGH_BITRATE)) {
			pApp->encParam[i].bitrate = HIGH_BITRATE;
		}
		else {
			pApp->encParam[i].bitrate = sEnc_Ch_Create_Params.u32Bit_Rate_Value;
		}

		switch (sEnc_Ch_Create_Params.eEnc_Codec_Profile)
		{
			case DBS_H264_BASELINE_PROFILE:
				pApp->encParam[i].profile=BASELINE_PROFILE;
				break;
			case DBS_H264_MAIN_PROFILE:
				pApp->encParam[i].profile=MAIN_PROFILE;
				break;
			case DBS_H264_HIGH_PROFILE:
				pApp->encParam[i].profile=HIGH_PROFILE;
				break;
			default:
				break;
		}

		switch (sEnc_Ch_Create_Params.eVideo_Codec)
		{
			case DBS_ENC_VIDEO_CODEC_H264:
				pApp->encParam[i].enc_type=AVC;
				break;
			case DBS_ENC_VIDEO_CODEC_H265:
				pApp->encParam[i].enc_type=HEVC;
				break;
			default:
				break;
		}

		switch (sEnc_Ch_Create_Params.eVAM_Video_In)
		{
			case DBS_ARINC_SRC_SVID_0:
				pApp->ipParam[i].device_type=TPG_1;
				break;
			case DBS_ARINC_SRC_SVID_1:
				pApp->ipParam[i].device_type=TPG_2;
				break;
			default:
				break;
		}

		switch (sEnc_Ch_Create_Params.eBit_Rate_Type)
		{
			case DBS_ENC_VBR:
			pApp->encParam[i].rc_mode=VBR;
				break;
			case DBS_ENC_CBR:
				pApp->encParam[i].rc_mode=CBR;
				break;
			default:
				break;
		}

		pApp->audParam[i].enable_audio = FALSE;


	//	ret = vgst_init ();
	//	if (ret !=  VGST_SUCCESS) {
	//		g_print("ERROR: vgst_init() failed error code %d :: \"%s\"\n", ret, vgst_error_to_string(ret, 0,channel));
	//		if (ret ==  VLIB_NO_MEDIA_SRC) {
	//			g_print("Get \"%s\" error in Tx only design, can be ignored!!!\n",  vgst_error_to_string(ret, 0,channel));
	//		} else {
	//			g_print("Error condition : CleanUp\n");
	//			return ret;
	//		}
	//	}

		/* create main context - each thread requires it's own context */
		pApp->loop_main_context = g_main_context_new ();
		if(NULL == pApp->loop_main_context)
		{
			goto CLEANUP;
		}

	    	/*-----------------------------------------------------------------------------*/
	    do {
			ret = vgst_config_options( pApp->encParam, pApp->ipParam, pApp->opParam, &pApp->cmnParam, pApp->audParam, channel);

			if(VGST_SUCCESS == ret) {
				ret = vgst_start_pipeline(channel);
					if(VGST_SUCCESS != ret) {
						g_print("ERROR: vgst_start_pipeline() failed error code \"%s\"\n", vgst_error_to_string(ret, 0,channel));
						goto QUIT;
					}
			} else {
				g_print("ERROR: vgst_config_options() failed, error code \"%s\"\n", vgst_error_to_string(ret, 0,channel));
				goto CLEANUP;
			}

			if (pApp->loop_playback)
			{
				g_timeout_add_seconds (pApp->loop_interval, (GSourceFunc)loop_cb, (gpointer)&pApp->channel);
			}
			pApp->loop = g_main_loop_new (pApp->loop_main_context, FALSE);
			g_main_loop_run(pApp->loop);
			g_print("Loop end.\n"
				"Playback count is %d.\n"
				"Loop_playback flag is %d.\n", ++count, pApp->loop_playback);
	    } while (pApp->loop_playback);

	QUIT :
		g_print("Quitting the pipeline.\n");
		ret = quit_pipeline (channel);
	    return ret;
	CLEANUP :
		vgst_app_deinit(channel);
	    return ret;
	}
	else
	{
		ret = ERROR;
	}
	return ret;
}

/**
 * \internal_start
 * \brief Stop encode
 *
 *
 * \sdd_start
 * <B>CSU Requirements and Constraints:</B>\n
 * \id_start vgst_app_stop_encode \id_end
 * \csu_start  \csu_end\n
 * \req_start \req_end\n
 *
 * <B>Algorithm & logic flow:</B>\n
 * - quit the pipeline
 * - free all resources
 * \sdd_end
 *
 * <B>Error handling & limitations:</B>\n
 *
 * \param guint channel - the required channel
 *
 * \return gint - 0 if succeed, other value if failed
 * \internal_end
 */
gint
vgst_app_stop_encode(guint channel)
{
	gint ret = 0;
	if((channel >= 0) && (channel < NUM_GST_CHANNELS))
	{
		App* pApp = &app_data[channel];

		// ODELYA - todo:change
		ret = quit_pipeline (channel);
		pApp->loop_playback = FALSE;
		if (pApp->loop && g_main_loop_is_running (pApp->loop)) {
			g_print ("Quitting the loop \n");
			g_main_loop_quit (pApp->loop);
			g_main_loop_unref (pApp->loop);
		}
	}
	else
	{
		ret = ERROR;
	}
    return ret;
}

/**
 * \internal_start
 * \brief main function for vcu-gst-app, for stand lone case
 *
 *
 * \sdd_start
 * <B>CSU Requirements and Constraints:</B>\n
 * \id_start main (stand alone case)/ vcu_gst_app_main \id_end
 * \csu_start  \csu_end\n
 * \req_start \req_end\n
 *
 * <B>Algorithm & logic flow:</B>\n
 * - set signal for ctrl+C
 * - call @vgst_app_init
 * - call @vgst_app_start_encode
 * - while loop playback is true:
 * 	- wait
 * - otherwise:
 * 	- call @vgst_app_deinit
 * \sdd_end
 *
 * <B>Error handling & limitations:</B>\n
 *
 * \param gint argc
 * \param gchar *argv[]
 *
 * \return gint - channel if succeed, other value if failed
 * \internal_end
 */
//#define STANDALONE
#if STANDALONE
gint
main (gint argc, gchar *argv[]) {
#else
gint
vcu_gst_app_main (gint argc, gchar *argv[]) {
#endif
    signal(SIGINT, signal_handler);
	int channel = vgst_app_init( TRUE, argc, argv);
	standalone_channal_number = channel;

	if((channel >= 0) && (channel < NUM_GST_CHANNELS))
	{
		App* pApp = &app_data[channel];

		vgst_app_start_encode(channel);
		while (pApp->loop_playback)
		{
			usleep(50000);
		}
		vgst_app_deinit(channel);
	}
    return channel;
}
