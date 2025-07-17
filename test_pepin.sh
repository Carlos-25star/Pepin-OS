#!/bin/bash

echo "Starting Pepin OS with Ext2 Filesystem..."
echo "This will run for 10 seconds to capture output..."
echo "=============================================="

# Run QEMU with our kernel and ext2 disk, capture output
timeout 10 qemu-system-i386 -kernel kernel -hda ext2_disk.img -nographic 2>&1 | head -100

echo ""
echo "=============================================="
echo "QEMU session completed"
echo "If you see Ext2 filesystem messages above, Phase 1 is working!"