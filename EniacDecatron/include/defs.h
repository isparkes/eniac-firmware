// ************************** Pin Allocations *************************

#define  Guide2_2   D4       // Guide 1 - G1 pin of 2-guide Dekatron
#define  Guide1_2   D3       // Guide 2 - G2 pin of 2-guide Dekatron
#define  Index2     D0       // Index   - NDX input pin. High when glow at K0

#define  Guide2_1   D6       // Guide 1 - G1 pin of 2-guide Dekatron
#define  Guide1_1   D5       // Guide 2 - G2 pin of 2-guide Dekatron
#define  Index1     D7       // Index   - NDX input pin. High when glow at K0

#define  HVEnable   D8
#define  inputPin1  RX

// The index mark is not at the "12 o'clock" position, but offset.
// We need to take this into account when calculating the expected position of the index mark.
#define DECATRON_1_TDC_OFFSET 27 
#define DECATRON_2_TDC_OFFSET 27
#define SPINNER_COUNTS_PER_STEP 1

// -------------------------------------------------------------------------------
#define SLAVE_DECA_MODE_MIN                  0
#define SLAVE_DECA_MODE_MINS_SECS            0
#define SLAVE_DECA_MODE_HOURS_MINS           1
#define SLAVE_DECA_MODE_SPINNER              2
#define SLAVE_DECA_MODE_OFF                  3
#define SLAVE_DECA_MODE_MAX                  3
#define SLAVE_DECA_MODE_DEFAULT              1

// -------------------------------------------------------------------------------
// Software version shown in config menu
#define SOFTWARE_VERSION      1

