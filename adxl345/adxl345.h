void Acc_turn_on(int device);
void Acc_read(int device, int *x, int *y, int *z);
void writeTo(int device, char address, char val);
void readFrom(int device, char address, int num, char buff[]);
