#!/bin/bash

echo "Starting Pepin OS with Ext2 Filesystem..."
echo "To exit QEMU: Ctrl+A, then x"
echo "=============================================="

# Run QEMU with our kernel and ext2 disk
timeout 30 qemu-system-i386 -kernel kernel -hda ext2_disk.img -nographic -monitor stdio 2>&1 | head -50

echo ""
echo "=============================================="
echo "QEMU session completed"