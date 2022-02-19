#ifndef ICON_DEFINES_H
#define ICON_DEFINES_H

// Note: all bytes, a-e have bit 7 set to '1'

// Byte a - Misc
#define K3_ICON_PRESET  (1>>)
#define K3_ICON_TXTEST  (1<<5)
#define K3_ICON_BSET    (1<<6)

// Byte b - Sub RX
#define K3_ICON_SUBRX_ON            (1<<0)   // 1 = ON
#define K3_ICON_SUBRX_NB            (1<<1)
#define K3_ICON_SUBRX_AUX           (1<<2)
#define K3_ICON_SUBRX_ANT           (1<<3)
#define K3_ICON_SUBRX_DIVRSTY       (1<<4)
#define K3_ICON_SUBRX_ABIND         (1<<5)
#define K3_ICON_SUBRX_VFOS_LINKED   (1<<6)

// Byte c - CW/Data
#define K3_ICON_TXT_2_TERM      (1<<0)
#define K3_ICON_SYNC_DATA       (1<<1)
#define K3_ICON_NORMAL_FSKTX    (1<<2)
#define K3_ICON_DUALTONE_FSK_FIL    (1<<3)
#define K3_ICON_VOX_ON_CW       (1<<4)
#define K3_ICON_DUALPB          (1<<5)
#define K3_ICON_QSKFULL         (1<<6)

// Byte d - Voice Modes
#define K3_ICON_MINUS_OFF       (1<<0)
#define K3_ICON_PLUS_OFF        (1<<1)
#define K3_ICON_FM_PL_TONE_ON   (1<<2)
#define K3_ICON_AM_SYNC_RX      (1<<3)
#define K3_ICON_NOISE_GATE_ON   (1<<4)
#define K3_ICON_ESSB            (1<<5)
#define K3_ICON_VOX_ON_SSB      (1<<6)

// Byte e - Misc
#define K3_ICON_SUBRX_NR_ON     (1<<3)
#define K3_ICON_MAINRX_SQUELCHED (1<<4)

#endif // ICON_DEFINES_H

