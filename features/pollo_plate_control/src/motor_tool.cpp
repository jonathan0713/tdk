#include <ros/ros.h>
#include <iostream>
#include <cstdint>
#include <fstream>
#include <chrono>
#include <thread>
#include <string>

extern "C"{
#include <motor_function.h>
}

void readRegister();
void writeRegister();
void readRegisterforspeed();
void settingController(uint8_t address);
void settingPID(uint8_t address, char type);
void clearMsg(serialData *targetMsg);

serialData msg;

int main(int argc, char **argv){
    ros::init(argc, argv, "motor_tool");
    ros::NodeHandle rosNh;

    

    int mode = 0;

    serialInit();

    while(ros::ok()){

        std::cout << "choose mode 1:read register 2:write register 3:setting controller 4:PID setting 5:moterspeed read" << std::endl;
        std::cin >> mode;
        std::cin.get();

        if(mode == 1){
            readRegister();
            CRC16Generate(&msg);
            transmitData(&msg);
            receiveData(&msg);
        }
        else if(mode == 2){
            writeRegister();
            CRC16Generate(&msg);
            transmitData(&msg);
            receiveData(&msg);

        }
        else if(mode == 3){
            int address;
            std::cout << "enter target address:" << std::endl;
            std::cin  >> std::dec >> address;
            std::cin.get();
            settingController((uint8_t)address);
        }
        else if(mode == 4){
            int address;
            char type;
            std::cout << "enter target address:" << std::endl;
            std::cin >> std::dec >> address;
            std::cin.get();
            std::cout << "p or i or d ?" << std::endl;
            std::cin >> type;
            std::cin.get();
            settingPID((uint8_t)address, type);
        }
        else if(mode == 5){
            readRegisterforspeed();
            
        }
        else{
            continue;
        }
    }


    return 0;
}

void readRegister(){
    int input = 0;

    clearMsg(&msg);
    msg.length = 8;
    std::cout << "enter controller address: (dec)" << std::endl;
    std::cin >> std::dec >> input;
    std::cin.get();
    msg.data[0] = (uint8_t)input;
    std::cout << std::endl;

    msg.data[1] = 0x03;

    std::cout << "enter register address: (hex)" << std::endl;
    std::cin >> std::hex >> input;
    std::cin.get();
    msg.data[3] = (0xff & input);
    msg.data[2] = (0xff & (input >> 8));
    std::cout << std::endl;
    std::cout << "enter numbers of registers: (dec)" << std::endl;
    std::cin >> std::dec >> input;
    std::cin.get();
    msg.data[5] = (0xff & input);
    msg.data[4] = (0xff & (input >> 8));

    std::cout << std::endl;

    return;
}

void readRegisterforspeed(){
    int input1 = 0;
    int input2 = 0;
    std::ofstream outputFile("/home/jonathan/Desktop/data.txt");

    clearMsg(&msg);
    msg.length = 8;
    std::cout << "enter controller address: (dec)" << std::endl;
    std::cin >> std::dec >> input1;
    std::cin.get();
    std::cout << "enter data: (dec)" << std::endl;
    std::cin >> std::dec >> input2;
    std::cin.get();
    msg.data[0] = (uint8_t)input1;
    msg.data[1] = 0x06;
    msg.data[3] = 0x43;
    msg.data[2] = 0x00;
    msg.data[5] = (0xff & input2);
    msg.data[4] = (0xff & (input2 >> 8));
    std::cout << std::endl;

    CRC16Generate(&msg);
    transmitData(&msg);
    receiveData(&msg);
    
    if (outputFile.is_open()) {
    outputFile << "X\tY\n";
    for (double x = 0.00; x <= 5.0; x += 0.01)
    {
    clearMsg(&msg);
    msg.length = 8;
    msg.data[0] = (uint8_t)input1;
    msg.data[1] = 0x03;
    msg.data[3] = 0x34;
    msg.data[2] = 0x00;
    msg.data[5] = 0x01;
    msg.data[4] = 0x00;
    std::cout << std::endl;

    CRC16Generate(&msg);
    transmitData(&msg);
    receiveData(&msg);

    // std::string element_3 = std::to_string(static_cast<int>(msg.data[3]));
    // std::string element_4 = std::to_string(static_cast<int>(msg.data[4]));

    // if(element_3.length() == 1) element_3 = "0" + element_3;
    // if(element_4.length() == 1) element_4 = "0" + element_4;
    // std::stringstream ss;
    // ss << element_3;
    // ss << element_4;

    // std::string element_combined = ss.str();
    // std::cout << element_combined << '\n';
    // return;

    // std::string element_filtered = element_combined.substr(2, 2) + element_combined.substr(6, 2);    

    uint8_t element_3 = msg.data[3];

    // 將 uint8_t 值轉換為四位數的十六進位數字字符串
    std::stringstream ss;
    ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(element_3);
    std::string hexString_3 = ss.str();

    // 將十六進位字符串擴展為四位
    hexString_3 += "00";

    // 將十六進位字符串轉換為十進位數
    int y1=0;
    std::stringstream(hexString_3) >> std::hex >> y1;
    
    unsigned char element_4 = msg.data[4];
    int y2 = (int)element_4;

    int y = y1+y2;

    outputFile << x << "\t" << y << "\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    }
        outputFile.close();
        std::cout << "Data has been written to data.txt" << std::endl;

    }

    // else {
    //     std::cerr << "Unable to open file for writing." << std::endl;
    //     return 1;
    // }
    clearMsg(&msg);
    msg.length = 8;
    msg.data[0] = (uint8_t)input1;
    msg.data[1] = 0x06;
    msg.data[3] = 0x43;
    msg.data[2] = 0x00;
    msg.data[5] = 0x00;
    msg.data[4] = 0x00;
    std::cout << std::endl;

    CRC16Generate(&msg);
    transmitData(&msg);
    receiveData(&msg);

    return;
}

void writeRegister(){
    int input;

    clearMsg(&msg);
    msg.length = 8;
    std::cout << "enter target address: (dec)" << std::endl;
    std::cin >> std::dec >> input;
    std::cin.get();
    msg.data[0] = (uint8_t)input;
    std::cout << std::endl;

    msg.data[1] = 0x06;

    std::cout << "enter register address: (hex)" << std::endl;
    std::cin >> std::hex >> input;
    std::cin.get();
    msg.data[3] = (0xff & input);
    msg.data[2] = (0xff & (input >> 8));
    std::cout << std::endl;

    std::cout << "enter data: (dec)" << std::endl;
    std::cin >> std::dec >> input;
    std::cin.get();
    msg.data[5] = (0xff & input);
    msg.data[4] = (0xff & (input >> 8));
    std::cout << std::endl;

    return ;
}

void settingController(uint8_t address){

    //0x6a = 0x271(625)
    clearMsg(&msg);
    msg.length = 8;
    msg.data[0] = address;
    msg.data[1] = 0x06;
    msg.data[3] = 0x6a;
    msg.data[4] = 0x02;
    msg.data[5] = 0x71; 
    CRC16Generate(&msg);
    transmitData(&msg);
    receiveData(&msg);

    //0x6b = 0x271(625)
    clearMsg(&msg);
    msg.length = 8;
    msg.data[0] = address;
    msg.data[1] = 0x06;
    msg.data[3] = 0x6b;
    msg.data[4] = 0x02;
    msg.data[5] = 0x71;
    CRC16Generate(&msg);
    transmitData(&msg);
    receiveData(&msg);

    //0x6c = 500
    clearMsg(&msg);
    msg.length = 8;
    msg.data[0] = address;
    msg.data[1] = 0x06;
    msg.data[3] = 0x6c;
    msg.data[4] = 0x01;
    msg.data[5] = 0xf4;
    CRC16Generate(&msg);
    transmitData(&msg);
    receiveData(&msg);

    //0x93 = 1
    clearMsg(&msg);
    msg.length = 8;
    msg.data[0] = address;
    msg.data[1] = 0x06;
    msg.data[3] = 0x93;
    msg.data[5] = 0x01;
    CRC16Generate(&msg);
    transmitData(&msg);
    receiveData(&msg);

    //0x94 = 0
    clearMsg(&msg);
    msg.length = 8;
    msg.data[0] = address;
    msg.data[1] = 0x06;
    msg.data[3] = 0x94;
    msg.data[5] = 0x00;
    CRC16Generate(&msg);
    transmitData(&msg);
    receiveData(&msg);

    //0x70 = 0
    clearMsg(&msg);
    msg.length = 8;
    msg.data[0] = address;
    msg.data[1] = 0x06;
    msg.data[3] = 0x70;
    msg.data[5] = 0x00;
    CRC16Generate(&msg);
    transmitData(&msg);
    receiveData(&msg);

    //0x79 = 0
    clearMsg(&msg);
    msg.length = 8;
    msg.data[0] = address;
    msg.data[1] = 0x06;
    msg.data[3] = 0x79;
    msg.data[5] = 0x00;
    CRC16Generate(&msg);
    transmitData(&msg);
    receiveData(&msg);

    //0x7a = 0
    clearMsg(&msg);
    msg.length = 8;
    msg.data[0] = address;
    msg.data[1] = 0x06;
    msg.data[3] = 0x7a;
    msg.data[5] = 0x00;
    CRC16Generate(&msg);
    transmitData(&msg);
    receiveData(&msg);

    //0x73 = 2
    clearMsg(&msg);
    msg.length = 8;
    msg.data[0] = address;
    msg.data[1] = 0x06;
    msg.data[3] = 0x73;
    msg.data[5] = 0x02;
    CRC16Generate(&msg);
    transmitData(&msg);
    receiveData(&msg);

    //0x74 = 10
    clearMsg(&msg);
    msg.length = 8;
    msg.data[0] = address;
    msg.data[1] = 0x06;
    msg.data[3] = 0x74;
    msg.data[5] = 0x0a;
    CRC16Generate(&msg);
    transmitData(&msg);
    receiveData(&msg);
    
    //0xe1 = 1
    clearMsg(&msg);
    msg.length = 8;
    msg.data[0] = address;
    msg.data[1] = 0x06;
    msg.data[3] = 0xe1;
    msg.data[5] = 0x01;
    CRC16Generate(&msg);
    transmitData(&msg);
    receiveData(&msg);

    return;
}

void settingPID(uint8_t address, char type){
    float floatinput;
    unsigned int *input_ptr;
    unsigned int  input;

    std::cout << "enter data: p(0.001~1) i(0.001~1) d(0.001~1) (dec)" << std::endl;
    std::cin >> std::dec >> floatinput;
    std::cin.get();

    input_ptr = (unsigned int *)&floatinput;
    input = *input_ptr;

    if(type == 'p'){
        clearMsg(&msg);
        msg.length = 8;
        msg.data[0] = address;
        msg.data[1] = 0x06;
        msg.data[2] = 0x00;
        msg.data[3] = 0xc0;
        msg.data[4] = (0xff & (input >> 24));
        msg.data[5] = (0xff & (input >> 16));
        CRC16Generate(&msg);
        transmitData(&msg);
        receiveData(&msg);

        clearMsg(&msg);
        msg.length = 8;
        msg.data[0] = address;
        msg.data[1] = 0x06;
        msg.data[2] = 0x00;
        msg.data[3] = 0xc1;
        msg.data[4] = (0xff & (input >> 8));
        msg.data[5] = (0xff & input);
        CRC16Generate(&msg);
        transmitData(&msg);
        receiveData(&msg);
    }
    else if(type == 'i'){
        clearMsg(&msg);
        msg.length = 8;
        msg.data[0] = address;
        msg.data[1] = 0x06;
        msg.data[2] = 0x00;
        msg.data[3] = 0xc2;
        msg.data[4] = (0xff & (input >> 24));
        msg.data[5] = (0xff & (input >> 16));
        CRC16Generate(&msg);
        transmitData(&msg);
        receiveData(&msg);

        clearMsg(&msg);
        msg.length = 8;
        msg.data[0] = address;
        msg.data[1] = 0x06;
        msg.data[2] = 0x00;
        msg.data[3] = 0xc3;
        msg.data[4] = (0xff & (input >> 8));
        msg.data[5] = (0xff & input);
        CRC16Generate(&msg);
        transmitData(&msg);
        receiveData(&msg);
    }
    else if(type == 'd'){
        clearMsg(&msg);
        msg.length = 8;
        msg.data[0] = address;
        msg.data[1] = 0x06;
        msg.data[2] = 0x00;
        msg.data[3] = 0xc4;
        msg.data[4] = (0xff & (input >> 24));
        msg.data[5] = (0xff & (input >> 16));
        CRC16Generate(&msg);
        transmitData(&msg);
        receiveData(&msg);

        clearMsg(&msg);
        msg.length = 8;
        msg.data[0] = address;
        msg.data[1] = 0x06;
        msg.data[2] = 0x00;
        msg.data[3] = 0xc5;
        msg.data[4] = (0xff & (input >> 8));
        msg.data[5] = (0xff & input);
        CRC16Generate(&msg);
        transmitData(&msg);
        receiveData(&msg);
    }
    
    return;
}

void clearMsg(serialData *targetMsg){
    for(int i = 0; i < 20; i++){
        targetMsg->data[i]= 0;
    }

    targetMsg->length = 0;

    return;
}