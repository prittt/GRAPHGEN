// Copyright(c) 2018 Costantino Grana, Federico Bolelli 
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met :
//
// *Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and / or other materials provided with the distribution.
//
// * Neither the name of GRAPHSGEN nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef GRAPHSGEN_RULESET_GENERATOR_H_
#define GRAPHSGEN_RULESET_GENERATOR_H_

#include "rule_set.h"

#define RSG_ALGO RSG(rosenfeld) RSG(rosenfeld_3d) RSG(bbdt) RSG(chen) RSG(ctbe) RSG(thin_zs) RSG(thin_gh) RSG(thin_ch) RSG(thin_hscp)

#define RSG(a) rule_set generate_##a();
RSG_ALGO
#undef RSG

#define RSG(a) a, 
enum class ruleset_generator_type { RSG_ALGO };
#undef RSG

#define RSG(a) #a, 
static std::string ruleset_generator_names[] = { RSG_ALGO };
#undef RSG

#define RSG(a) generate_##a, 
static rule_set(*ruleset_generator_functions[])(void) = { RSG_ALGO };
#undef RSG

#endif // !GRAPHSGEN_RULESET_GENERATOR_H_