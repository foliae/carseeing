#pragma once

#include <ctime>
#include<chrono>
#include "aes.h"
#include "xcb_rng.hpp"

#include "parser.h"

#define SIZE_MAX_PC_ID 50
#define SIZE_MAX_AUTH_CODE 10
#define SIZE_MAX_EMBED_ID 50  //不要超过25
#define SIZE_MAX_CHECK_CODE 10
#define SIZE_MAX_COMPANY 20
#define SIZE_MAX_PROJECT 20    //order num
#define SIZE_MAX_ORDER_PROFILE 500

void xcb_auth_backend_data_init(xcb_auth_backend_data_t &data);


bool decrypt_data_from_pc_to_backend(const unsigned char *data_cipher,
                                     int nData, xcb_auth_backend_data_t &data);

bool encrypt_data_from_backend_to_pc(unsigned char *data_encrypt, int &nData,
                                     const xcb_auth_backend_data_t &data);
