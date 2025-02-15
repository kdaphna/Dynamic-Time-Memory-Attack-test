#define CIPHER_NAME "Simon_16"

uint32_t z_0 = 0b00011001110000110101001000101111;

//AS ALMOST IN ALL VERSION OF SIMON, WORD SIZE ARE OF MULTIPLE OF 8/16
//I WILL USE uint8 and uint16 DATA TYPE AND TYPE COVERSION BETWEEN THEM

uint8_t rotate_left(uint8_t x, int n){
	uint8_t y = ( ((x) << (n)) | ((x) >> (8-(n))) );
	return y;
}  

uint8_t rotate_right(uint8_t x, int n){
	uint8_t y = ( ((x) >> (n)) | ((x) << (8-(n))) );
	return y;
}    

void key_schedule ( uint8_t *key ){
	int i;
    uint8_t temp;

    for(i=4 ; i<32 ; i++){
        temp = rotate_right(key[i-1],3);
        temp = temp ^ key[i-3];
        temp = temp ^ rotate_right(temp,1);
        key[i] = ~key[i-4] ^ temp ^ z_0 ^ 3;
    }
}
     
void encrypt(const uint8_t *key, const uint8_t *plaintext, uint8_t *ciphertext){
    int i;
    uint8_t temp;
    uint8_t y;
    uint8_t x;
    
    //plaintext = {x,y}, y is least significant word & R(x,y)=(y^f(x)^k,x)
    y = plaintext[0];
    x = plaintext[1];		

    for(i = 0; i < 32; i++){
        temp= (rotate_left(x, 1) & rotate_left(x, 2)) ^ rotate_left(x, 3); // f(x)
        temp ^= y;
        y = x;
        x = temp ^ key[i]; //Add Round Key
    }
    ciphertext[1] = y;  //No interchange in last stage
    ciphertext[0] = x;
}

void decrypt(const uint8_t *key, const uint8_t *ciphertext, uint8_t *plaintext){
	int i;
    uint8_t temp;
	uint8_t y;
    uint8_t x;
    y = ciphertext[0];
    x = ciphertext[1];

    for(i = 0; i < 32; i++){
        temp = (rotate_left(x, 1) & rotate_left(x, 5)) ^ rotate_left(x, 2); // f(x)
        temp ^= y;
        y = x;
        x = temp ^ key[31 - i]; //Add Round Key
    }
    plaintext[1] = y;  //No interchange in last stage
    plaintext[0] = x;
}

uint32_t evaluate_key(const uint32_t master_key)
{
    uint32_t plaintext = 0x83828180;
	uint8_t roundkeys[32] = {0};
    uint8_t p[4], p_encrypted[4];
    
    memcpy(p, &plaintext, 4);

    // copy K into the least significant part of the roundkeys
    memcpy(roundkeys, &master_key, 4);
	key_schedule(roundkeys);
	
	// p[0] = 0x80;
	// p[1] = 0x81;
	encrypt(roundkeys, p, p_encrypted);
	     
    // p[2] = 0x82;
    // p[3] = 0x83;
    encrypt(roundkeys, p + 2, p_encrypted + 2);
    
	return *((uint32_t*) p_encrypted); // return a 32-bit integer
}
