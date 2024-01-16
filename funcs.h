size_t float1_to_str(float f, uint8_t *to);
ssize_t ucp2utf8(uint32_t cp, uint8_t *buf);
#define UCP_MAX 0x110000
void byte2xcode(uint8_t b, uint8_t **p);
