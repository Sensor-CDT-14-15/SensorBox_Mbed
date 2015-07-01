#include "mbed.h"
#include "nRF24L01P.h"
#include "MMA8451Q.h"
#include <vector>
#include <math.h>

#define MMA8451_I2C_ADDRESS (0x1d<<1)
#define TRANSFER_SIZE   24
#define ARRAY_LENGTH    20

Serial pc(USBTX, USBRX); // tx, rx

PinName const SDA = PTE25;
PinName const SCL = PTE24;

AnalogIn light_ain(A0);
AnalogIn temp_ain(A1);
AnalogIn pir_ain(A3);
AnalogIn sound_ain(A4);
AnalogIn noise_ain(A5);

nRF24L01P my_nrf24l01p(PTD2, PTD3, PTD1, PTE1, PTE0, PTD0);    // mosi, miso, sck, csn, ce, irq

DigitalOut myled1(LED1);
DigitalOut myled2(LED2);

uint16_t light;
uint16_t temp;
uint16_t pir;
uint16_t noise;

std::vector<uint16_t> light_data;
std::vector<uint16_t> temp_data;
std::vector<uint16_t> pir_data;
std::vector<uint16_t> noise_data;

void getdata()
{
    light = light_ain.read_u16();
    temp = temp_ain.read_u16();
    pir = pir_ain.read_u16();
    noise = noise_ain.read_u16();

    light_data.push_back (light);
    temp_data.push_back (temp);
    pir_data.push_back (pir);
    noise_data.push_back (noise);

    light_data.pop_back();
    temp_data.pop_back();
    pir_data.pop_back();
    noise_data.pop_back();

    printf("%f\n", light_data);
}

void average()
{
    int light_sum = 0;
    int temp_sum = 0;
    int noise_sum = 0;
    float light_avg, temp_avg, noise_avg, light_var, temp_var, noise_var;
    float light_residuals =0; 
    float temp_residuals = 0 ; 
    float noise_residuals = 0; 
 
    for (int i = 0; i < ARRAY_LENGTH; i++) {
        light_sum += light_data[i];
        temp_sum += temp_data[i];
        noise_sum += noise_data[i];
    }
    light_avg = light_sum / (float)ARRAY_LENGTH;
    temp_avg = temp_sum / (float)ARRAY_LENGTH;
    noise_avg = noise_sum / (float)ARRAY_LENGTH;
    
    /*  Compute  variance  and standard deviation  */
    for (int i = 0; i < ARRAY_LENGTH; i++) {
       light_residuals += pow((light_data[i] - light_avg), 2);
       temp_residuals += pow((temp_data[i] - temp_avg), 2);      
       noise_residuals += pow((noise_data[i] - noise_avg), 2);       
    }
    light_var = light_residuals/(float)ARRAY_LENGTH;
    temp_var = temp_residuals/(float)ARRAY_LENGTH;
    noise_var = noise_residuals/(float)ARRAY_LENGTH;
    
    printf("Light Average = %.2f\n", light_avg);
    printf("Temp Average = %.2f\n", temp_avg);
    printf("Noise Average = %.2f\n", noise_avg);
}



int main()
{

    MMA8451Q acc(SDA, SCL, MMA8451_I2C_ADDRESS);

    light_data.assign (ARRAY_LENGTH,0);
    temp_data.assign (ARRAY_LENGTH,0);
    pir_data.assign (ARRAY_LENGTH,0);


    char txData[TRANSFER_SIZE], rxData[TRANSFER_SIZE];
    int txDataCnt = 0;
    int rxDataCnt = 0;

    my_nrf24l01p.powerUp();

    // Display the (default) setup of the nRF24L01+ chip
    //pc.printf( "nRF24L01+ Frequency    : %d MHz\r\n",  my_nrf24l01p.getRfFrequency() );
    //pc.printf( "nRF24L01+ Output power : %d dBm\r\n",  my_nrf24l01p.getRfOutputPower() );
    //pc.printf( "nRF24L01+ Data Rate    : %d kbps\r\n", my_nrf24l01p.getAirDataRate() );
    //pc.printf( "nRF24L01+ TX Address   : 0x%010llX\r\n", my_nrf24l01p.getTxAddress() );
    //pc.printf( "nRF24L01+ RX Address   : 0x%010llX\r\n", my_nrf24l01p.getRxAddress() );

    //pc.printf( "Type keys to test transfers:\r\n  (transfers are grouped into %d characters)\r\n", TRANSFER_SIZE );

    my_nrf24l01p.setTransferSize( TRANSFER_SIZE );

    my_nrf24l01p.setReceiveMode();
    my_nrf24l01p.enable();

    printf("MMA8451 ID: %d\n", acc.getWhoAmI());

    while (1) {
        getdata();
        average();
        // txDataCnt =  sprintf(txData, "  %1.3f   %1.3f   %1.3f\n", x,y,z);
        //my_nrf24l01p.write( NRF24L01P_PIPE_P0, txData, txDataCnt );
        myled1 = !myled1;
        wait(0.5);
    }
}


