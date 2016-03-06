#pragma once
#include "hufftree.h"

using namespace hcp;

void huffEncode( bitArray &bA, CRITICAL_SECTION &susp );
void huffDecode( bitArray &bA, CRITICAL_SECTION &susp );