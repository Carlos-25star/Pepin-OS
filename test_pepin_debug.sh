#!/bin/bash

echo "Starting Pepin OS with Ext2 Filesystem..."
echo "This will run for 15 seconds to capture output..."
echo "=============================================="

# Run QEMU with our kernel and ext2 disk, specify raw format to avoid warnings
timeout 15 qemu-system-i386 -kernel kernel -hda ext2_disk.img -nographic -no-reboot -d cpu_reset,pcall 2>&1 | head -200

echo ""
echo "=============================================="
echo "QEMU session completed"