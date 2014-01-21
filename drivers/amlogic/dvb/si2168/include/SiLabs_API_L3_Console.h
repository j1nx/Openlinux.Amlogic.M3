#ifndef  _SiLabs_API_L3_Console_H_
#define  _SiLabs_API_L3_Console_H_

#include "SiLabs_API_L3_Wrapper.h"

  /* At project level, define the TER_TUNER_Si21xx value corresponding to the required terrestrial tuner */
 #ifndef   Si2146_L3_TEST_H
  #ifdef    TER_TUNER_Si2146
    #include         "Si2146_L3_Test.h"
  #endif /* TER_TUNER_Si2146 */
 #endif /* Si2146_L3_TEST_H */

 #ifndef   Si2156_L3_TEST_H
  #ifdef    TER_TUNER_Si2156
    #include         "Si2156_L3_Test.h"
  #endif /* TER_TUNER_Si2156 */
 #endif /* Si2156_L3_TEST_H */

 #ifndef   Si2173_L3_TEST_H
  #ifdef    TER_TUNER_Si2173
    #include         "Si2173_L3_Test.h"
  #endif /* TER_TUNER_Si2173 */
 #endif /* Si2173_L3_TEST_H */

 #ifndef   Si2176_L3_TEST_H
  #ifdef    TER_TUNER_Si2176
    #include         "Si2176_L3_Test.h"
  #endif /* TER_TUNER_Si2176 */
 #endif /* Si2176_L3_TEST_H */

 #ifndef   Si2178_L3_TEST_H
  #ifdef    TER_TUNER_Si2178
    #include         "Si2178_L3_Test.h"
  #endif /* TER_TUNER_Si2178 */
 #endif /* Si2178_L3_TEST_H */

 #ifndef   Si2190_L3_TEST_H
  #ifdef    TER_TUNER_Si2190
    #include         "Si2190_L3_Test.h"
  #endif /* TER_TUNER_Si2190 */
 #endif /* Si2190_L3_TEST_H */

 #ifndef   Si2196_L3_TEST_H
  #ifdef    TER_TUNER_Si2196
    #include         "Si2196_L3_Test.h"
  #endif /* TER_TUNER_Si2196 */
 #endif /* Si2196_L3_TEST_H */


  /* At project level, define the TER_TUNER_Si21xx value corresponding to the required terrestrial tuner */
 #ifndef   Si2146_L3_TEST_H
  #ifdef    TER_TUNER_Si2146
    #include         "Si2146_L3_Test.h"
  #endif /* TER_TUNER_Si2146 */
 #endif /* Si2146_L3_TEST_H */

 #ifndef   Si2156_L3_TEST_H
  #ifdef    TER_TUNER_Si2156
    #include         "Si2156_L3_Test.h"
  #endif /* TER_TUNER_Si2156 */
 #endif /* Si2156_L3_TEST_H */

 #ifndef   Si2173_L3_TEST_H
  #ifdef    TER_TUNER_Si2173
    #include         "Si2173_L3_Test.h"
  #endif /* TER_TUNER_Si2173 */
 #endif /* Si2173_L3_TEST_H */

 #ifndef   Si2176_L3_TEST_H
  #ifdef    TER_TUNER_Si2176
    #include         "Si2176_L3_Test.h"
  #endif /* TER_TUNER_Si2176 */
 #endif /* Si2176_L3_TEST_H */

 #ifndef   Si2190_L3_TEST_H
  #ifdef    TER_TUNER_Si2190
    #include         "Si2190_L3_Test.h"
  #endif /* TER_TUNER_Si2190 */
 #endif /* Si2190_L3_TEST_H */

 #ifndef   Si2196_L3_TEST_H
  #ifdef    TER_TUNER_Si2196
    #include         "Si2196_L3_Test.h"
  #endif /* TER_TUNER_Si2196 */
 #endif /* Si2196_L3_TEST_H */


#ifdef    __cplusplus
 extern "C" {
#endif /* __cplusplus */

typedef struct SiLabs_Carriers_Struct {
  int            carriers_count ;
  int           *standard       ;
  int           *freq           ;
  int           *bandwidth_Hz   ;
  int           *stream         ;
  unsigned int  *symbol_rate_bps;
  int           *constellation  ;
  int           *polarization   ;
  int           *band           ;
  int           *plp_id         ;
} SiLabs_Carriers_Struct;

extern SiLabs_Carriers_Struct *Carriers_Table;

int   SiLabs_Scan_Table_Init          (void);
int   SiLabs_Scan_Table_Clear         (void);
int   SiLabs_Scan_Table_Count         (void);
int   SiLabs_Scan_Table_AddOneCarrier (                   int  standard, int  freq, int  bandwidth_Hz, int  stream, unsigned int  symbol_rate_bps, int  constellation, int  polarization, int  band, int  plp_id);
int   SiLabs_Scan_Table_Carrier_Info  (int carrier_index, int *standard, int *freq, int *bandwidth_Hz, int *stream, unsigned int *symbol_rate_bps, int *constellation, int *polarization, int *band, int *plp_id);
int   SiLabs_Scan_Table_Carrier_Text  (int carrier_index, char *formatted_infos);
int   SiLabs_Scan_Table_Infos         (void);

#ifdef    __cplusplus
}
#endif /* __cplusplus */

#endif /* _SiLabs_API_L3_Console_H_ */

