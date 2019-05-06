# Lab X

## Extra Credit

This is an extra credit lab. You are not required to complete this lab for your regular grade, and the TAs may or may not answer questions about this lab. Furthermore, this lab is out of 0, but you may earn up to 100 points.

## Assignment

You will be writing a VIO (Virtual I/O) driver which probes, sets up, and reads from block and entropy (random) devices.

## Functions You Need To Write

All of the functions you need to write are in labx.cpp. You will notice that I've defined several structures for you in vio.h (and data types). Take a look at the top of labx.cpp. There are a few utilities, such as write_string you may use.

```cpp
void lab_x_test()
```

This function is free for you to use as you see fit. This will be called whenever you type 0 at the prompt when the kernel is running. The liblabx.a does not call any of your functions except this one. So, if you want to test your entire program, you need to probe, setup, and read via this function.

```cpp
DeviceType vio_x_probe(uint32_t base)
```

This function will probe a given base address for the existence of a block or entropy device. The memory locations available to you are 0x1000_0000 up to (and including) 0x1000_8000. Each device occupies 4KiB of memory. You must examine the MAGIC and DeviceId (in the MmioRegs enumeration) and make sure that the magic is correct.

This function does NOT set up the device. Instead, it just checks to see if it exists. If it doesn't, return DeviceType::Invalid, otherwise return DeviceType::[Entropy, Block].

### Probing

Probing for devices is a two step process. First, since this is MMIO, you need to check the MAGIC (offset 0) of each MMIO address. A valid device has a 32-bit magic number of 0x74_72_69_76 (which spells "virt" in little endian). So, if this number appears, then you need to make the next step -- check the DeviceId. The DeviceId of a block device is 2, and the DeviceId of an entropy device is 4. There are others, but you will need to return Invalid if it is not a block or entropy device.

```cpp
bool vio_x_setup(uint32_t base, DeviceType type)
```

This function sets up a device at base of type, type. This is responsible for negotiating with the device, setting up the virtual queues, and making sure the device is in the ready state.

### Setting Up Devices

Chapter 3.1.1 shows how to initialize a device. The bits, such as ACKNOWLEDGE are listed in the VioDeviceStatus enumeration in vio.h. Setting up block devices is in chapter 5.2.5 of which the FeatureBits are enumerated in vio.h as VioBlkFeatureBits, and setting up entropy devices is in chapter 5.4.5 (very easy, nothing needs to be done except set up the virtqueue).

You will notice that step #7 shows that you need to set up the "virt queues". These virt queues are part of a structure VQueue also in vio.h. These queues are used to send/receive data to/from the virtual device.

```cpp
void vio_x_read_block(uint32_t base, uint32_t start_sector, char *buffer)
```

This function reads from the device at base. This assumes that base is a block device, and it has been setup by vio_x_setup.

### Reading From Block Device

The virt queue is a structure in memory. The address of the VQueue structure needs to go into the QueuePfn MMIO register. This tells the virtual device where the queue is located. After you fill the queue, you need to notify the device by writing which queue to use (typically 0) in the QueueNotify MMIO register.

You can get the drive details from the Config (0x100) register. This will populate an entire VirtioBlkConfig structure.

Then, you will need to submit a request through your virt queue. To do this, you will need to create a packet. The packet consists of: Block Head (BlockRequestHead structure), Data buffer (BlockRequestData structure), Status buffer (BlockRequestStatus structure).

When you create this buffer, you need to then set up the virtual queue to point to this structure in memory. You do this by setting the address in the VQueueDesc to the address of the top of the packet. The len field will be the size of the entire packet. The flags field will be NEXT and the next field will be the index to the next queue. Each read or write has three block requests: 1. The block head, 2. The data buffer, and 3. The status buffer. If you want to write to the device, you must make sure that the VIRTQ_DESC_F_WRITE flag is set in the VQueueDesc. The status is written to by the virtual device, which is one of the fields in the BlockRequestStatusFlags enumeration.

Finally, you need to set the available ring to the index of the queue descriptor that you're going to use. This available ring will contain three queue numbers (indices) which will represent the block head, data buffer, and status buffer.

After this, send the buffer via QueueNotify, but writing the queue's index here (typically 0). The device will process and return via the InterruptStatus register (0x060). Essentially, you need to take a snapshot of this register, notify the device, and then wait for this register to change. When the register changes, this notifies you that the device responded. Typically, this would go through an interrupt, but to keep things simple, we're polling it.

The block device itself will populate BlockRequestData with the actual data, and it will put the status of the read in the BlockRequestStatus field. If the status is Ok (0), then you can assume the BlockRequestData is good to go.

By the way, each request reads one sector at a time (512 bytes).

```cpp
uint32_t vio_x_read_random(uint32_t base)
```

This function reads from the device at base. This assumes that base is an entropy device, and it has been properly setup by vio_x_setup. You will need to set up the virtual queues like you did in the block devices. However, when you submit work, all you need to do is provide a VQueue structure. The address you put into VQueue::addr will be where the random number generator will place a value. Just like the block device, you will be notified that the device is finished via the InterruptStatus register.

## Files

```
vio_export.zip
```

## Testing

```bash
make run
```

This command will compile your program, create a block device (filled with random values), and then execute the virtual machine.

## Hints and Restrictions

1. You may need to use statics (global variables). This is OK.

2. You will mostly be graded on results (did it work, as in produce a random number or produce block data?) rather than coding style, but please make sure you format your code properly and make it readable.

3. Please comment your logic so we can give you maximum points.

4. All virtqueues have a queue size of 1024. I was going to have you query the size, which is why there is a template class for the size. But, being that I'm such a generous instructor, you can assume 1024 for all sizes.

## Submission

Submit your .cpp file.

### References

[virtio-v1.0-cs04.pdf](virtio-v1.0-cs04.pdf)

Virtual Queues: Chapter 2.4

Device Initialization: Chapter 3.1

Device Operation: Chapter 3.2

MMIO Transport: Chapter 4.2.4 (Legacy Interface)

Block Device: Chapter 5.2

Entropy Device: Chapter 5.4