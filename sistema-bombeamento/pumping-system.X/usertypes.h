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
    MIN_LEVEL
} screen_mode;

typedef struct
{
    float level;
    float min_level;
    float max_level;
} tank;