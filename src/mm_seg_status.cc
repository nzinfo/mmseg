/*
 * Copyright 2014 Li Monan <limn@coreseek.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 */

#include "mm_seg_status.h"

namespace mm {

SegStatus::SegStatus(u4 size) {

}

void SegStatus::Reset() {

}

int SegStatus::MoveNext() {
	return 0;
}

bool SegStatus::IsPause() {
	return false;
}

const DictMatchResult* SegStatus::GetMatchesAt(u4 pos, u2* count) {
	return NULL;
}

u2 SegStatus::SetTagA(u4 pos, u2 tag) {
	return 0;
}

u2 SegStatus::SetTagB(u4 pos, u2 tag) {
	return 0;
}

u2 SegStatus::SetTagPush(u4 pos, u2 tag) {
	return 0;
}

void SegStatus::BuildTermIndex() {

}

} // namespace mm
