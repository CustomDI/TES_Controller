#define RETURN_IF_ERROR(expr) do { \
    uint8_t _s = (expr);           \
    if (_s) return _s;             \
} while(0)