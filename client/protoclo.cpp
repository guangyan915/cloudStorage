#include "protoclo.h"
#include <QDebug>

// 使用智能指针来管理

std::unique_ptr<DataPack, FreeDeleter> makeAddFriendPack(unsigned int request_size) {
    unsigned int total_size = sizeof(DataPack) + sizeof(MessagePack) + sizeof(AddFriendPack) + request_size;
    std::unique_ptr<DataPack, FreeDeleter> pack((DataPack*)std::malloc(total_size));
    if(pack == nullptr) {
        exit(EXIT_FAILURE);
    }
    memset(pack.get(), 0, total_size);
    MessagePack* msg = (MessagePack*)pack->pack_data;
    msg->data_size = sizeof(AddFriendPack) + request_size;
    pack->total_size = total_size;
    pack->pack_type = PACK_TYPE_MSG;
    return pack;
}

std::unique_ptr<DataPack, FreeDeleter> makeDataPack(unsigned int pack_size) {
    unsigned int total_size = sizeof(DataPack) + pack_size;
    std::unique_ptr<DataPack, FreeDeleter> pack((DataPack*)std::malloc(total_size));
    if(pack == nullptr) {
        exit(EXIT_FAILURE);
    }
    memset(pack.get(), 0, total_size);
    pack->total_size = total_size;
    return pack;
}

std::unique_ptr<DataPack, FreeDeleter> makeDataPackMsg(unsigned int data_size)
{
    std::unique_ptr<DataPack, FreeDeleter> pack = makeDataPack(data_size + sizeof(MessagePack));
    if(pack == nullptr) {
        exit(EXIT_FAILURE);
    }
    pack->pack_type = PACK_TYPE_MSG;
    MessagePack* msg_page = (MessagePack*)pack->pack_data;
    msg_page->data_size = data_size;
    return pack;
}

std::unique_ptr<DataPack, FreeDeleter> makeDataPackFile(unsigned int data_size)
{
    std::unique_ptr<DataPack, FreeDeleter> pack = makeDataPack(data_size + sizeof(FilePack));
    if(pack == nullptr) {
        exit(EXIT_FAILURE);
    }
    pack->pack_type = PACK_TYPE_UPLOAD_FILE;
    return pack;
}
