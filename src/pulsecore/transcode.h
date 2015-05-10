#ifndef footranscodehfoo
#define footranscodehfoo

#include <pulsecore/core-format.h>




typedef enum pa_transcode_flags {
         PA_TRANSCODE_DECODER=(1<<0),
         PA_TRANSCODE_ENCODER=(1<<1)
         } pa_transcode_flags_t;

typedef struct pa_transcode {
 int32_t encoding;
 uint8_t channels;
 uint32_t frame_size;
 uint32_t max_frame_size;
 uint32_t sample_size;
 uint32_t rate;
 pa_transcode_flags_t flags;
 union {
         void *decoder;
         void *encoder;
 };
} pa_transcode;

bool pa_transcode_supported(pa_encoding_t encoding);
void *pa_transcode_create_decoder(pa_encoding_t encoding);
void pa_transcode_init(pa_transcode *transcode, pa_encoding_t encoding, pa_transcode_flags_t flags, pa_format_info *transcode_format_info, pa_sample_spec *transcode_sink_spec);
void pa_transcode_free(pa_transcode *transcode);
int32_t pa_transcode_encode(pa_transcode *transcode, unsigned char *pcm_input, unsigned char **compressed_output);
int32_t pa_transcode_decode(pa_transcode *transcode, unsigned char *compressed_input, int input_length, unsigned char *pcm_output);

#endif
