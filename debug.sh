cd build
gdb --quiet -ex bt -ex quit ./src/RpcClient core

gdb --quiet \
    -ex start \
    -ex "watch _az_log_message_callback" \
    -ex "watch _az_message_filter_callback" \
    -ex continue \
    -ex bt \
    -ex disassemble \
    -batch \
    ./src/RpcClient


 