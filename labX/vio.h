#pragma once
using uint64_t = unsigned long long;
using uint32_t = unsigned int;
using uint16_t = unsigned short;
using uint8_t  = unsigned char;
using int64_t  = signed long long;
using int32_t  = signed int;
using int16_t  = signed short;
using int8_t   = signed char;
// VIO device types
// There are more than these, but the library
// only supports entropy and block.
enum DeviceType
{
	Invalid = 0,
	Block = 2,
	Entropy = 4,
};

const uint32_t VIO_MAGIC = 0x74726976;

enum MmioRegs {
	MagicValue = 0x000,
	Version = 0x004,
	DeviceId = 0x008,
	VendorId = 0x00c,
	HostFeatures = 0x010,
	HostFeaturesSel = 0x014,
	GuestFeatures = 0x020,
	GuestFeaturesSel = 0x024,
	GuestPageSize = 0x028,
	QueueSel = 0x030,
	QueueNumMax = 0x034,
	QueueNum = 0x038,
	QueueAlign = 0x03c,
	QueuePfn = 0x040,
	QueueNotify = 0x050,
	InterruptStatus = 0x060,
	InterruptAck = 0x064,
	Status = 0x070,
	Config = 0x100,
};

enum VioDeviceStatus : uint32_t {
        Reset = 0,
        Acknowledge = 1,
        Driver = 2,
        DriverOk = 4,
        FeaturesOk = 8,
        DeviceNeedsReset = 64,
        Failed = 128
};

enum VioBlkFeatureBits {
	SizeMax = 1,
	SegMax = 2,
	Geometry = 4,
	ReadOnly = 5,
	BlkSize = 6,
	BlkFlush = 9,
	Topology = 10,
	ConfigWce = 11,

	Barrier = 0,
	Scsi = 7,
};

struct VirtioBlkGeometry {
	uint16_t cylinders;
	uint8_t  heads;
	uint8_t  sectors;
};

struct VirtioBlkTopology {
	uint8_t physical_block_exp;
	uint8_t alignment_offset;
	uint16_t min_io_size;
	uint32_t opt_io_size;
};

struct VirtioBlkConfig {
	uint64_t capacity;
	uint32_t size_max;
	uint32_t seg_max;
	VirtioBlkGeometry geometry;
	uint32_t blk_size;
	VirtioBlkTopology topology;
	uint8_t writeback;
};

struct VQueueDesc {
#define VIRTQ_DESC_F_NEXT     1
#define VIRTQ_DESC_F_WRITE    2
#define VIRTQ_DESC_F_INDIRECT 4
	uint64_t addr;
	uint32_t len;
	uint16_t flags;
	uint16_t next;
};

template<int SZ=1024>
struct VQueueAvail {
#define VIRTQ_AVAIL_F_NO_INTERRUPT 1
	uint16_t flags;
	uint16_t idx;
	uint16_t ring[SZ];
	uint16_t used_event;
};

struct VQueueUsedElem {
	uint32_t id;
	uint32_t len;
};

template<int SZ=1024>
struct VQueueUsed {
#define VIRTQ_USED_F_NO_NOTIFY 1
	uint16_t flags;
	uint16_t idx;
	VQueueUsedElem ring[SZ];
	uint16_t avail_event;
};

template<int SZ=1024>
struct VQueue {
	VQueueDesc desc[SZ];
	VQueueAvail<SZ> avail;
	VQueueUsed<SZ>  used;
};

// Block device stuff
enum BlockRequestType : uint32_t {
	In = 0,
	Out = 1,
	Flush = 4,
};

enum BlockRequestStatusFlags : uint8_t {
	Uninitialized = 111,
	Ok = 0,
	IoErr = 1,
	Unsupp = 2
};

struct BlockRequestHead {
	BlockRequestType type;
	uint32_t reserved;
	uint64_t sector;
};

struct BlockRequestData {
	uint8_t data[512];
};

struct BlockRequestStatus {
	BlockRequestStatusFlags status;
};

extern "C" {
        // in labx.cpp
	void lab_x_test();
        DeviceType vio_x_probe(uint32_t base);
	bool vio_x_setup(uint32_t base, DeviceType type);
	void vio_x_read_block(uint32_t base, uint32_t start_sector, char *buffer);
	uint32_t vio_x_read_random(uint32_t base);
}

