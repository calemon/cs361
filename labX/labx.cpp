#include <vio.h>

// Helper functions written for you:
extern "C" {
        void write_string(const char *s);
        void write_stringln(const char *s);
        void dec_to_string(char *dest, int value);
        void hex_to_string(char *dest, int hex_value, bool prefix=true);
        void strcpy(char *dest, const char *src);
}

// This function is called whenever you type 0 from the menu prompt.
// NOTICE: This may run multiple times!
void lab_x_test()
{
	write_stringln("\r\nLab X Test");
}

// vio_x_probe, probes an MMIO base address
// to see if a device exists here.
// This returns the device type if found or DeviceType::Invalid
// if nothing was probed.
DeviceType vio_x_probe(uint32_t base)
{
        return DeviceType::Invalid;
}

// vio_x_setup, sets up an MMIO device
// of type, type. If the base was able to be setup
// properly, it will return true, otherwise it returns
// false.
bool vio_x_setup(uint32_t base, DeviceType type)
{
        return false;
}

// This assumes the buffer is at least 512 bytes.
void vio_x_read_block(uint32_t base, uint32_t start_sector, char *buffer)
{

}

// Reads a random number and returns it.
uint32_t vio_x_read_random(uint32_t base)
{
        return 0;
}
