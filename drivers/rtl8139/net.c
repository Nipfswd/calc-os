#include <video.h>
#include <utils.h>
#include <idt.h>
#include <stdint.h>
#include <pci.h>
#include <mm.h>

volatile uint8_t* rtl_mmio;

#define RX_BUFFER_SIZE 0x4000

// Вирівнювання за стандартами RTL8139
uint8_t rtl_rx_buffer[RX_BUFFER_SIZE] __attribute__((aligned(16)));
volatile uint8_t rtl_tx_buffer[2048] __attribute__((aligned(4)));

static uint8_t tx_counter = 0;

int rtl8139_find() {
    for (int i = 0; i < device_count; i++) {
        if (devices[i].vendor_id == 0x10EC && devices[i].device_id == 0x8139) {
            return i;
        }
    }
    return -1;
}

uint32_t rtl8139_get_mmio() {
    uint32_t mmio_base;

    int index = rtl8139_find();
    if (index < 0) return 0;

    // BAR1 (offset 0x14) відповідає за Memory Space BAR
    mmio_base = pci_read_config_dword(devices[index].bus, devices[index].slot, devices[index].func, 0x14);
    mmio_base = mmio_base & ~0xF;

    return mmio_base;
}

void rtl8139_init() {
    uint32_t rx_phys = (uint32_t)rtl_rx_buffer;

    int index = rtl8139_find();
    if (index < 0) return;

    // Вмикаємо Bus Mastering + MMIO в командному регістрі PCI
    uint16_t cmd = pci_read_config_word(devices[index].bus, devices[index].slot, devices[index].func, 0x04);
    cmd = cmd | 0x0007; 
    pci_write_config_word(devices[index].bus, devices[index].slot, devices[index].func, 0x04, cmd);

    uint32_t mmio_base = rtl8139_get_mmio();
    if (mmio_base == 0) return;
    rtl_mmio = (volatile uint8_t*)mmio_base;

    // 1. Software Reset
    rtl_mmio[0x37] = 0x10;
    uint32_t timeout = 500000;
    while ((rtl_mmio[0x37] & 0x10) && timeout > 0) {
        timeout--;
    }
    if (timeout == 0) return;

    // 2. Конфігурація приймача (Приймаємо Broadcast, Multicast, My physical, Promiscuous)
    *(volatile uint32_t*)(rtl_mmio + 0x44) = 0x0000E70F;

    // 3. Передаємо адресу RX буфера
    *(volatile uint32_t*)(rtl_mmio + 0x30) = rx_phys;

    // 4. Скидаємо лічильники кадру
    *(volatile uint16_t*)(rtl_mmio + 0x38) = 0;
    *(volatile uint16_t*)(rtl_mmio + 0x3A) = 0xFFFF;

    // 5. Налаштування переривань (TOK та ROK)
    *(volatile uint16_t*)(rtl_mmio + 0x3C) = 0x0005;

    // 6. Вмикаємо передавач (TE) та приймач (RE)
    rtl_mmio[0x37] = 0x0C;

    tx_counter = 0;
}

uint8_t read_pack() {
    // Перевірка прапорця Buffer Empty (Rx Buffer Empty)
    if (rtl_mmio[0x37] & 0x01) {
        return 0;
    }

    uint16_t read_ptr = *(volatile uint16_t*)(rtl_mmio + 0x3A);
    uint16_t real_ptr = (read_ptr + 16) & (RX_BUFFER_SIZE - 1);
    
    if (read_ptr == 0xFFFF) {
        real_ptr = 0;
    }

    uint8_t* header = rtl_rx_buffer + real_ptr;

    uint16_t status = header[0] | (header[1] << 8);
    if (!(status & 0x01)) { // Перевірка біта ROK (Receive OK) у заголовку
        return 0;
    }

    uint16_t length = header[2] | (header[3] << 8);

    uint8_t* payload = header + 4;
    
    // БЕЗПЕКА: Перевірка типу кадру (EtherType) перед тим, як лізти глибоко в offset
    uint16_t ethertype = (payload[12] << 8) | payload[13];
    
    // Якщо це ваш кастомний пакет (0x88B5), виводимо сирий символ
    if (ethertype == 0x88B5) {
        char c = payload[14];
        put_char(c, 1);
    } 
    // Якщо це ICMP/IP пакет (наприклад, від ping), використовуємо ваш старий offset
    else if (length >= 42) {
        uint16_t offset = 14 + 20 + 8;
        uint8_t* icmp_data = payload + offset;
        for (int i = 0; i < length - offset - 4; i++) {
            char c = icmp_data[i];
            put_char(c, 1);
        }
    }

    // Оновлюємо покажчик кільцевого буфера за специфікацією Realtek Dword alignment
    real_ptr = (real_ptr + length + 4 + 3) & ~3;
    *(volatile uint16_t*)(rtl_mmio + 0x3A) = real_ptr - 16;

    return 1;
}

void send_pack(uint8_t data, uint8_t dest_mac[6]) {
    uint32_t phys_tx_buf = (uint32_t)rtl_tx_buffer; 

    // Формуємо Ethernet-заголовок
    for (int i = 0; i < 6; i++) {
        rtl_tx_buffer[i] = dest_mac[i];
    }
    
    // Читаємо наш власний MAC з MAC-регістрів карти (IDR0-IDR5)
    for (int i = 0; i < 6; i++) {
        rtl_tx_buffer[i + 6] = rtl_mmio[i];
    }

    // EtherType: Локальний експериментальний протокол
    rtl_tx_buffer[12] = 0x88;
    rtl_tx_buffer[13] = 0xB5;

    // Payload
    rtl_tx_buffer[14] = data;

    // Padding (мінімальний кадр Ethernet — 60 байт)
    for (int i = 15; i < 60; i++) {
        rtl_tx_buffer[i] = 0;
    }

    uint32_t length = 60; 
    uint8_t tx_reg_offset = tx_counter * 4;

    // Безпечний запис адреси через каст до точного типу
    volatile uint32_t* tsad_reg = (volatile uint32_t*)(rtl_mmio + 0x10 + tx_reg_offset);
    *tsad_reg = phys_tx_buf;

    __asm__ __volatile__("" : : : "memory");

    // Безпечний запис довжини (ініціація трансляції DMA)
    volatile uint32_t* tsd_reg = (volatile uint32_t*)(rtl_mmio + 0x00 + tx_reg_offset);
    *tsd_reg = length; // Важливо: біт 13 (TOK) автоматично скидається в 0 при старті

    // Безпечне опитування біта статусу (БЕЗ нелінійного кастування на льоту)
    // Очікуємо біт 13 (TOK - Transmit OK) або біт 14 (TUN - Transmit Underrun)
    while (!(*tsd_reg & (1 << 13))) {
        // Очікування завершення DMA
    }

    // Переходимо до наступного з 4-х доступних TX дескрипторів
    tx_counter = (tx_counter + 1) % 4;
}