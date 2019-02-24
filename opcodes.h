


#define MOVOPCODE (0)
#define CMPOPCODE (0|(1<<6))
#define ADDOPCODE (0|(2<<6))
#define SUBOPCODE (0|(3<<6))
#define NOTOPCODE (0|(4<<6))
#define CLROPCODE (0|(5<<6))
#define LEAOPCODE (0|(6<<6))
#define INCOPCODE (0|(7<<6))
#define DECOPCODE (0|(8<<6))
#define JMPOPCODE (0|(9<<6))
#define BNEOPCODE (0|(10<<6))
#define REDOPCODE (0|(11<<6))
#define PRNOPCODE (0|(12<<6))
#define JSROPCODE (0|(13<<6))
#define RTSOPCODE (0|(14<<6))
#define STOPOPCODE (0|(15<<6))




extern Bitoper destoperarr[];
extern Bitoper souoperarr[];
extern Bitoper grparr[];
extern Bitoper rndarr[];
extern Bitoper AREarr[];
extern Bitoper regs[];
extern Guiding guide[];
extern Operation operarr[];

