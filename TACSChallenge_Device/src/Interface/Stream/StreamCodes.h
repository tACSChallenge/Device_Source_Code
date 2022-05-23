#ifndef _STREAM_CODES_H_
#define _STREAM_CODES_H_


enum SC {SC_UNDEF = 0,
         SC_END = 0,
         SC_STIM_ID,
         SC_RB_RESP_TIME,
         SC_MODE,
         SC_ADC_LASTTIME,
         SC_ECHT_LASTTIME,
         SC_ADC_DT,
         SC_ECHT_DT,
         SC_IN_SIGNAL_RAW,
         SC_IN_SIGNAL_EEG,
         SC_IN_SIGNAL,
         SC_IN_SIGNAL_NODC,
         SC_IN_SIGNAL_NODC_FILT,
         SC_OUT_SIGNAL,
         SC_INST_AMP,
         SC_INST_PHASE,
         SC_PRESS,
         SC_BREATH,
         SC_ACCEL,
         SC_FFT_BIN_FREQ_WIDTH,
         SC_WINDOW_DUR,
         SC_ADC0,
         SC_ADC1,
         SC_RSB,
         SC_DATA1, /* EXAMPLE - Only used in SlowWaveExperiment.h. Delete when not needed. */
         SC_DATA2, /* EXAMPLE - Only used in SlowWaveExperiment.h. Delete when not needed. */
         SC_DATA3, /* EXAMPLE - Only used in SlowWaveExperiment.h. Delete when not needed. */
				 SC_SW_MEAN,
         /* ADD NEW STREAMING CODES ABOVE HERE! */
         SC_SIZE_
        };


#endif // _STREAM_CODES_H_
