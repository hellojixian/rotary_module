#include "clock_verify.h"

void verify_clock(void) {
    Serial.println("=== Clock Verification ===");

    // 打印CPU频率信息
    print_clock_info();

    // 验证频率设置
    uint32_t cpu_freq = get_current_cpu_frequency();
    Serial.print(F("Current CPU Frequency: "));
    Serial.print(cpu_freq / 1000000);
    Serial.println(F(" MHz"));

    // 打印时钟源信息
    Serial.print(F("Clock Source: "));
    Serial.println(get_clock_source_name());

    Serial.println(F("========================="));
}

void print_clock_info(void) {
    #if ((F_CPU) == 1000000)
    #warning F_CPU = 1MHz
    Serial.println(F("Configured F_CPU: 1MHz"));
    #elif ((F_CPU) == 2000000)
    #warning F_CPU = 2MHz
    Serial.println(F("Configured F_CPU: 2MHz"));
    #elif ((F_CPU) == 4000000)
    #warning F_CPU = 4MHz
    Serial.println(F("Configured F_CPU: 4MHz"));
    #elif ((F_CPU) == 8000000)
    #warning F_CPU = 8MHz
    Serial.println(F("Configured F_CPU: 8MHz"));
    #elif ((F_CPU) == 16000000)
    #warning F_CPU = 16MHz
    Serial.println(F("Configured F_CPU: 16MHz"));
    #elif ((F_CPU) == 32000000)
    #warning F_CPU = 32MHz
    Serial.println(F("Configured F_CPU: 32MHz"));
    #else
    Serial.print(F("Configured F_CPU: "));
    Serial.print(F_CPU / 1000000);
    Serial.println(F(" MHz (Custom)"));
    #endif
}

uint32_t get_current_cpu_frequency(void) {
    return F_CPU;
}

const char* get_clock_source_name(void) {
    #ifdef CLOCK_SOURCE
        #if ((CLOCK_SOURCE) == 1)
        return "Internal Clock";
        #elif ((CLOCK_SOURCE) == 2)
        return "External Clock";
        #else
        return "Unknown Clock Source";
        #endif
    #else
        return "Clock Source Not Defined";
    #endif
}
