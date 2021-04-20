typedef unsigned char uint8;

typedef enum
{ 
    AUTOMATIC, 
    MANUAL
} operation_mode;

typedef enum
{ 
    CURRENT_LEVEL, 
    OPERATION_MODE,
    MIN_LEVEL,
    MODBUS
} screen_mode;

typedef struct
{
    float level;
    float min_level;
    float max_level;
} tank;

typedef struct
{
    char line0 [16];
    char line1 [16];
} lcd;