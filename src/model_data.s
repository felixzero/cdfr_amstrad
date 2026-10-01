.globl _obstacles
.globl _obstacle_flags

NUMBER_OF_OBSTACLES = 20

.area _INITIALIZED

_obstacles:
    ; W-E oriented quarries
    .db  4, -5, 14, 10
    .db 26, -5, 14, 10
    .db  4, 53, 14, 10
    .db 26, 53, 14, 10
    .db 74, -5, 14, 10
    .db 52, -5, 14, 10
    .db 74, 53, 14, 10
    .db 52, 53, 14, 10
    ; N-S oriented quarries
    .db 38, 11, 10, 14
    .db 39, 38, 10, 14
    ; W-E player 1 walls
    .db -3,  8, 14, 10
    .db 13,  8, 14, 10
    .db -4, 41, 14, 10
    .db 12, 41, 14, 10
    ; Player 1 gate
    .db 16, 25, 10, 14
    ; W-E player 2 walls
    .db 80,  7, 14, 10
    .db 65,  8, 14, 10
    .db 80, 41, 14, 10
    .db 66, 41, 14, 10
    ; Player 2 gate
    .db 61, 24, 10, 14
    ; Player 1 towers
    .db  9, 10,  6,  6
    .db  8, 43,  6,  6
    .db 21, 18,  6,  6
    .db 21, 36,  6,  6
    ; Player 2 towers
    .db 73, 10,  6,  6
    .db 76, 43,  6,  6
    .db 64, 15,  6,  6
    .db 65, 36,  6,  6

_obstacle_flags:
    .db 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x82, 0x82
    .db 0x04, 0x04, 0x04, 0x04, 0x84
    .db 0x05, 0x05, 0x05, 0x05, 0x85
    .db 0x08, 0x08, 0x08, 0x08
    .db 0x09, 0x09, 0x09, 0x09

