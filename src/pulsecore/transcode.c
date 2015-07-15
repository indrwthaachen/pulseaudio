
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <pulsecore/macro.h>
#include <pulsecore/core-format.h>


#include "transcode.h"



#ifdef HAVE_OPUS

#include <opus.h>

#define OPUS_DEFAULT_FRAME_SIZE 2880
#define OPUS_DEFAULT_BITRATE 64000

#define OPUS_DEFAULT_SAMPLE_RATE 48000
#define OPUS_DEFAULT_SAMPLE_SIZE 2



OpusEncoder *tc_opus_create_encoder(int sample_rate, int channels, int bitrate);
OpusDecoder *tc_opus_create_decoder(int sample_rate, int channels);
int tc_opus_encode(OpusEncoder *encoder, unsigned char *pcm_bytes, unsigned char *cbits, int channels, int frame_size);
int tc_opus_decode(OpusDecoder *decoder, unsigned char *cbits, int nbBytes, unsigned char *pcm_bytes, int channels, int max_frame_size);


OpusDecoder *tc_opus_create_decoder(int sample_rate, int channels)
{
   int err;

   OpusDecoder * decoder = opus_decoder_create(sample_rate, channels, &err);
   if (err<0) {
      pa_log_error("failed to create decoder: %s", opus_strerror(err));
      return NULL;
   }

   return decoder;
}



OpusEncoder *tc_opus_create_encoder(int sample_rate, int channels, int bitrate)
{
   int err;

   OpusEncoder *encoder = opus_encoder_create(sample_rate, channels, OPUS_APPLICATION_AUDIO, &err);
   if (err<0){
      pa_log_error("failed to create an encoder: %s", opus_strerror(err));
      return NULL;
   }

   err = opus_encoder_ctl(encoder, OPUS_SET_BITRATE(bitrate));
   if (err<0)
   {
      pa_log_error("failed to set bitrate: %s", opus_strerror(err));
      return NULL;
   }

   return encoder;
}


int tc_opus_encode(OpusEncoder *encoder, unsigned char *pcm_bytes, unsigned char *cbits, int channels, int frame_size)
{
      int i;
      int nbBytes;
      opus_int16 in[frame_size*channels];

      /* Convert from little-endian ordering. */
      for (i=0;i<channels*frame_size;i++)
         in[i]=pcm_bytes[2*i+1]<<8|pcm_bytes[2*i];


      nbBytes = opus_encode(encoder, in, frame_size, cbits, frame_size*channels*2);
      if (nbBytes<0)
      {
         pa_log_error("encode failed: %s", opus_strerror(nbBytes));
         return EXIT_FAILURE;
      }

      return nbBytes;
}


int tc_opus_decode(OpusDecoder *decoder, unsigned char *cbits, int nbBytes, unsigned char *pcm_bytes, int channels, int max_frame_size)
{
      int err=0, i;
      opus_int16 out[max_frame_size*channels];

      int frame_size = opus_decode(decoder, cbits, nbBytes, out, max_frame_size, 0);
      if (frame_size<0)
      {
         pa_log_error("decoder failed: %s", opus_strerror(err));
         return EXIT_FAILURE;
      }

      /* Convert to little-endian ordering. */
      for(i=0;i<channels*frame_size;i++)
      {
         pcm_bytes[2*i]=out[i]&0xFF;
         pcm_bytes[2*i+1]=(out[i]>>8)&0xFF;
      }

      return frame_size;
}

#endif


bool pa_transcode_supported(pa_encoding_t encoding) {
         #ifdef HAVE_OPUS
         if(encoding == PA_ENCODING_OPUS)
                  return 1;
         #endif

         return 0;
}

void pa_transcode_set_format_info(pa_transcode *transcode, pa_format_info *f) {
         pa_format_info_set_prop_int(f, "frame_size", transcode->frame_size);
         pa_format_info_set_prop_int(f, "bitrate", transcode->bitrate);
}


void pa_transcode_init(pa_transcode *transcode, pa_encoding_t encoding, pa_transcode_flags_t flags, pa_format_info *transcode_format_info, pa_sample_spec *transcode_sink_spec) {

         pa_assert((flags & PA_TRANSCODE_DECODER) || (flags & PA_TRANSCODE_ENCODER));

         transcode->flags = flags;
         transcode->encoding = encoding;

         switch(encoding) {
                  #ifdef HAVE_OPUS
                  case PA_ENCODING_OPUS:

                           transcode->sample_size = OPUS_DEFAULT_SAMPLE_SIZE;


                           if(flags & PA_TRANSCODE_DECODER) {
                                    if(pa_format_info_get_prop_int(transcode_format_info, "max_frame_size", (int *)&transcode->max_frame_size) != 0)
                                             transcode->max_frame_size = OPUS_DEFAULT_FRAME_SIZE;
                                    if(pa_format_info_get_prop_int(transcode_format_info, "frame_size", (int *)&transcode->frame_size) != 0)
                                             transcode->frame_size = OPUS_DEFAULT_FRAME_SIZE;
                                    if(pa_format_info_get_prop_int(transcode_format_info, "bitrate", (int *)&transcode->bitrate) != 0)
                                             transcode->bitrate = OPUS_DEFAULT_BITRATE;

                                    pa_format_info_get_rate(transcode_format_info, &transcode->rate);
                                    pa_format_info_get_channels(transcode_format_info, &transcode->channels);

                                    transcode->decoder = tc_opus_create_decoder(transcode->rate, transcode->channels);

                           }
                           else {
                                    if(transcode->bitrate == 0)
                                             transcode->bitrate = OPUS_DEFAULT_BITRATE;
                                    if(transcode->frame_size == 0)
                                             transcode->frame_size = OPUS_DEFAULT_FRAME_SIZE;

                                    transcode->channels = transcode_sink_spec->channels;
                                    transcode->rate = OPUS_DEFAULT_SAMPLE_RATE;
                                    transcode->encoder = tc_opus_create_encoder(transcode->rate, transcode->channels, transcode->bitrate);


                                    transcode_sink_spec->rate = transcode->rate;
                                    transcode_sink_spec->format = PA_SAMPLE_S16LE;
                           }

                           break;
                  #endif
                  default:
                           transcode->decoder = NULL;
                           transcode->encoder = NULL;
         }

}

void pa_transcode_free(pa_transcode *transcode) {

         switch(transcode->encoding) {
                  #ifdef HAVE_OPUS
                  case PA_ENCODING_OPUS:
                           opus_decoder_destroy(transcode->decoder);
                           break;
                  #endif
                  default:
                           transcode->decoder = NULL;
         }

}


int32_t pa_transcode_encode(pa_transcode *transcode, unsigned char *pcm_input, unsigned char **compressed_output) {
         int nbBytes=0;

         switch(transcode->encoding) {
                  #ifdef HAVE_OPUS
                  case PA_ENCODING_OPUS:
                           *compressed_output = malloc(transcode->frame_size*transcode->channels*transcode->sample_size);
                           nbBytes = tc_opus_encode(transcode->encoder, pcm_input, *compressed_output, transcode->channels, transcode->frame_size);
                           break;
                  #endif

         }

         return nbBytes;
}

int32_t pa_transcode_decode(pa_transcode *transcode, unsigned char *compressed_input, int input_length, unsigned char *pcm_output) {
         int32_t frame_length=0;

         switch(transcode->encoding) {
                  #ifdef HAVE_OPUS
                  case PA_ENCODING_OPUS:
                           frame_length = tc_opus_decode(transcode->decoder, compressed_input, input_length, pcm_output, transcode->channels, transcode->max_frame_size);
                           break;
                  #endif

         }

         return frame_length;
}
