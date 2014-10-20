#ifndef DYPTH01_H_
#define DYPTH01_H_

void TH01_InitPort(int pin_sdi, int pin_sdo, int pin_sck, int pin_ss_n);
int TH01_ReadData(int *t, int *hr);

#endif // !DYPTH01_H_
