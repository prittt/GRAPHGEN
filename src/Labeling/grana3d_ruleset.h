// Copyright(c) 2018 - 2019 Costantino Grana, Federico Bolelli 
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

#include <string>

#include "graphgen.h"

constexpr auto Kh = 0;
constexpr auto Lg = 1;
constexpr auto Lh = 2;
constexpr auto Mg = 3;
constexpr auto Nf = 4;
constexpr auto Nh = 5;
constexpr auto Oe = 6;
constexpr auto Of = 7;
constexpr auto Og = 8;
constexpr auto Oh = 9;
constexpr auto Pe = 10;
constexpr auto Pg = 11;
constexpr auto Qf = 12;
constexpr auto Re = 13;
constexpr auto Rf = 14;
constexpr auto Se = 15;
constexpr auto Td = 16;
constexpr auto Th = 17;
constexpr auto Uc = 18;
constexpr auto Ud = 19;
constexpr auto Ug = 20;
constexpr auto Uh = 21;
constexpr auto Vc = 22;
constexpr auto Vg = 23;
constexpr auto Wb = 24;
constexpr auto Wd = 25;
constexpr auto Wf = 26;
constexpr auto Wh = 27;
constexpr auto Xa = 28;
constexpr auto Xb = 29;
constexpr auto Xc = 30;
constexpr auto Xd = 31;
constexpr auto Xe = 32;
constexpr auto Xf = 33;
constexpr auto Xg = 34;
constexpr auto Xh = 35;

constexpr auto K = 0;
constexpr auto L = 1;
constexpr auto M = 2;
constexpr auto N = 3;
constexpr auto O = 4;
constexpr auto P = 5;
constexpr auto Q = 6;
constexpr auto R = 7;
constexpr auto S = 8;
constexpr auto T = 9;
constexpr auto U = 10;
constexpr auto V = 11;
constexpr auto W = 12;
constexpr auto x = 13;

//template<int rule>
action_bitset GetActions(rule_wrapper& r, 
	const rule_set& rs, 
	bool K_set, 
	bool L_set,
	bool M_set,
	bool N_set,
	bool O_set, 
	bool P_set,
	bool Q_set, 
	bool R_set, 
	bool S_set, 
	bool T_set, 
	bool U_set, 
	bool V_set, 
	bool W_set) {
	//const bool K = ((rule >> 0) & 1) == 1;
	//const bool L = ((rule >> 1) & 1) == 1;
	//const bool M = ((rule >> 2) & 1) == 1;
	//const bool N = ((rule >> 3) & 1) == 1;
	//const bool O = ((rule >> 4) & 1) == 1;
	//const bool P = ((rule >> 5) & 1) == 1;
	//const bool Q = ((rule >> 6) & 1) == 1;
	//const bool R = ((rule >> 7) & 1) == 1;
	//const bool S = ((rule >> 8) & 1) == 1;
	//const bool T = ((rule >> 9) & 1) == 1;
	//const bool U = ((rule >> 10) & 1) == 1;
	//const bool V = ((rule >> 11) & 1) == 1;
	//const bool W = ((rule >> 12) & 1) == 1;

	connectivity_mat con({ "K","L","M","N","O","P","Q","R","S","T","U","V","W","x" });

	// X connections
	if (K_set) con.set(x, K, r[Xa] && r[Kh]);
	if (L_set) con.set(x, L, (r[Lg] || r[Lh]) && (r[Xa] || r[Xb]));
	if (M_set) con.set(x, M, r[Xb] && r[Mg]);
	if (N_set) con.set(x, N, (r[Xa] || r[Xc]) && (r[Nf] || r[Nh]));
	if (O_set) con.set(x, O, ((r[Xa] || r[Xb] || r[Xc] || r[Xd]) && (r[Oe] || r[Of] || r[Og] || r[Oh])));
	if (P_set) con.set(x, P, (r[Xb] || r[Xd]) && (r[Pe] || r[Pg]));
	if (Q_set) con.set(x, Q, r[Qf] && r[Xc]);
	if (R_set) con.set(x, R, (r[Xc] || r[Xd]) && (r[Re] || r[Rf]));
	if (S_set) con.set(x, S, r[Se] && r[Xd]);
	if (T_set) con.set(x, T, (r[Xa] || r[Xe]) && (r[Td] || r[Th]));
	if (U_set) con.set(x, U, ((r[Xa] || r[Xb] || r[Xe] || r[Xf]) && (r[Uc] || r[Ud] || r[Ug] || r[Uh])));
	if (V_set) con.set(x, V, (r[Xb] || r[Xf]) && (r[Vc] || r[Vg]));
	if (W_set) con.set(x, W, ((r[Xa] || r[Xc] || r[Xe] || r[Xg]) && (r[Wd] || r[Wb] || r[Wh] || r[Wf])));


	// Adjacent Block Connections
	if (L_set && K_set) con.set(K, L, (r[Kh]) && (r[Lg]));
	if (N_set && K_set) con.set(K, N, (r[Kh]) && (r[Nf]));
	if (O_set && K_set) con.set(K, O, (r[Kh]) && (r[Oe]));
	if (T_set && K_set) con.set(K, T, (r[Kh]) && (r[Td]));
	if (U_set && K_set) con.set(K, U, (r[Kh]) && (r[Uc]));
	if (W_set && K_set) con.set(K, W, (r[Kh]) && (r[Wb]));
	if (M_set && L_set) con.set(L, M, (r[Lh]) && (r[Mg]));
	if (N_set && L_set) con.set(L, N, (r[Lg]) && (r[Nf]));
	if (O_set && L_set) con.set(L, O, (r[Lg] || r[Lh]) && (r[Oe] || r[Of]));
	if (P_set && L_set) con.set(L, P, (r[Lh]) && (r[Pe]));
	if (T_set && L_set) con.set(L, T, (r[Lg]) && (r[Td]));
	if (U_set && L_set) con.set(L, U, (r[Lg] || r[Lh]) && (r[Uc] || r[Ud]));
	if (V_set && L_set) con.set(L, V, (r[Lh]) && (r[Vc]));
	if (W_set && L_set) con.set(L, W, (r[Lg]) && (r[Wb]));
	if (O_set && M_set) con.set(M, O, (r[Mg]) && (r[Of]));
	if (P_set && M_set) con.set(M, P, (r[Mg]) && (r[Pe]));
	if (U_set && M_set) con.set(M, U, (r[Mg]) && (r[Ud]));
	if (V_set && M_set) con.set(M, V, (r[Mg]) && (r[Vc]));
	if (O_set && N_set) con.set(N, O, (r[Nf] || r[Nh]) && (r[Oe] || r[Og]));
	if (Q_set && N_set) con.set(N, Q, (r[Nh]) && (r[Qf]));
	if (R_set && N_set) con.set(N, R, (r[Nh]) && (r[Re]));
	if (T_set && N_set) con.set(N, T, (r[Nf]) && (r[Td]));
	if (U_set && N_set) con.set(N, U, (r[Nf]) && (r[Uc]));
	if (W_set && N_set) con.set(N, W, (r[Nf] || r[Nh]) && (r[Wb] || r[Wd]));
	if (P_set && O_set) con.set(O, P, (r[Of] || r[Oh]) && (r[Pe] || r[Pg]));
	if (Q_set && O_set) con.set(O, Q, (r[Og]) && (r[Qf]));
	if (R_set && O_set) con.set(O, R, (r[Og] || r[Oh]) && (r[Re] || r[Rf]));
	if (S_set && O_set) con.set(O, S, (r[Oh]) && (r[Se]));
	if (T_set && O_set) con.set(O, T, (r[Oe]) && (r[Td]));
	if (U_set && O_set) con.set(O, U, (r[Oe] || r[Of]) && (r[Uc] || r[Ud]));
	if (V_set && O_set) con.set(O, V, (r[Of]) && (r[Vc]));
	if (W_set && O_set) con.set(O, W, (r[Oe] || r[Og]) && (r[Wb] || r[Wd]));
	if (R_set && P_set) con.set(P, R, (r[Pg]) && (r[Rf]));
	if (S_set && P_set) con.set(P, S, (r[Pg]) && (r[Se]));
	if (U_set && P_set) con.set(P, U, (r[Pe]) && (r[Ud]));
	if (V_set && P_set) con.set(P, V, (r[Pe]) && (r[Vc]));
	if (R_set && Q_set) con.set(Q, R, (r[Qf]) && (r[Re]));
	if (W_set && Q_set) con.set(Q, W, (r[Qf]) && (r[Wd]));
	if (S_set && R_set) con.set(R, S, (r[Rf]) && (r[Se]));
	if (W_set && R_set) con.set(R, W, (r[Re]) && (r[Wd]));
	if (U_set && T_set) con.set(T, U, (r[Td] || r[Th]) && (r[Uc] || r[Ug]));
	if (W_set && T_set) con.set(T, W, (r[Td] || r[Th]) && (r[Wb] || r[Wf]));
	if (V_set && U_set) con.set(U, V, (r[Ud] || r[Uh]) && (r[Vc] || r[Vg]));
	if (W_set && U_set) con.set(U, W, (r[Uc] || r[Ug]) && (r[Wb] || r[Wf]));

	// Non-adjacent, transitive Block Connections
	bool changed = true;
	while (changed) {
		changed = false;

		if (K_set && L_set) changed |= con.set(K, L, con(K, L) || ((con(K, N) && con(N, L)) || (con(K, O) && con(O, L)) || (con(K, T) && con(T, L)) || (con(K, U) && con(U, L)) || (con(K, W) && con(W, L))));
		if (K_set && M_set) changed |= con.set(K, M, con(K, M) || ((con(K, L) && con(L, M)) || (con(K, N) && con(N, M)) || (con(K, O) && con(O, M)) || (con(K, T) && con(T, M)) || (con(K, U) && con(U, M)) || (con(K, W) && con(W, M))));
		if (K_set && N_set) changed |= con.set(K, N, con(K, N) || ((con(K, L) && con(L, N)) || (con(K, O) && con(O, N)) || (con(K, T) && con(T, N)) || (con(K, U) && con(U, N)) || (con(K, W) && con(W, N))));
		if (K_set && O_set) changed |= con.set(K, O, con(K, O) || ((con(K, L) && con(L, O)) || (con(K, N) && con(N, O)) || (con(K, T) && con(T, O)) || (con(K, U) && con(U, O)) || (con(K, W) && con(W, O))));
		if (K_set && P_set) changed |= con.set(K, P, con(K, P) || ((con(K, L) && con(L, P)) || (con(K, N) && con(N, P)) || (con(K, O) && con(O, P)) || (con(K, T) && con(T, P)) || (con(K, U) && con(U, P)) || (con(K, W) && con(W, P))));
		if (K_set && Q_set) changed |= con.set(K, Q, con(K, Q) || ((con(K, L) && con(L, Q)) || (con(K, N) && con(N, Q)) || (con(K, O) && con(O, Q)) || (con(K, T) && con(T, Q)) || (con(K, U) && con(U, Q)) || (con(K, W) && con(W, Q))));
		if (K_set && R_set) changed |= con.set(K, R, con(K, R) || ((con(K, L) && con(L, R)) || (con(K, N) && con(N, R)) || (con(K, O) && con(O, R)) || (con(K, T) && con(T, R)) || (con(K, U) && con(U, R)) || (con(K, W) && con(W, R))));
		if (K_set && S_set) changed |= con.set(K, S, con(K, S) || ((con(K, L) && con(L, S)) || (con(K, N) && con(N, S)) || (con(K, O) && con(O, S)) || (con(K, T) && con(T, S)) || (con(K, U) && con(U, S)) || (con(K, W) && con(W, S))));
		if (K_set && T_set) changed |= con.set(K, T, con(K, T) || ((con(K, L) && con(L, T)) || (con(K, N) && con(N, T)) || (con(K, O) && con(O, T)) || (con(K, U) && con(U, T)) || (con(K, W) && con(W, T))));
		if (K_set && U_set) changed |= con.set(K, U, con(K, U) || ((con(K, L) && con(L, U)) || (con(K, N) && con(N, U)) || (con(K, O) && con(O, U)) || (con(K, T) && con(T, U)) || (con(K, W) && con(W, U))));
		if (K_set && V_set) changed |= con.set(K, V, con(K, V) || ((con(K, L) && con(L, V)) || (con(K, N) && con(N, V)) || (con(K, O) && con(O, V)) || (con(K, T) && con(T, V)) || (con(K, U) && con(U, V)) || (con(K, W) && con(W, V))));
		if (K_set && W_set) changed |= con.set(K, W, con(K, W) || ((con(K, L) && con(L, W)) || (con(K, N) && con(N, W)) || (con(K, O) && con(O, W)) || (con(K, T) && con(T, W)) || (con(K, U) && con(U, W))));
		if (L_set && M_set) changed |= con.set(L, M, con(L, M) || ((con(L, K) && con(K, M)) || (con(L, N) && con(N, M)) || (con(L, O) && con(O, M)) || (con(L, P) && con(P, M)) || (con(L, T) && con(T, M)) || (con(L, U) && con(U, M)) || (con(L, V) && con(V, M)) || (con(L, W) && con(W, M))));
		if (L_set && N_set) changed |= con.set(L, N, con(L, N) || ((con(L, K) && con(K, N)) || (con(L, M) && con(M, N)) || (con(L, O) && con(O, N)) || (con(L, P) && con(P, N)) || (con(L, T) && con(T, N)) || (con(L, U) && con(U, N)) || (con(L, V) && con(V, N)) || (con(L, W) && con(W, N))));
		if (L_set && O_set) changed |= con.set(L, O, con(L, O) || ((con(L, K) && con(K, O)) || (con(L, M) && con(M, O)) || (con(L, N) && con(N, O)) || (con(L, P) && con(P, O)) || (con(L, T) && con(T, O)) || (con(L, U) && con(U, O)) || (con(L, V) && con(V, O)) || (con(L, W) && con(W, O))));
		if (L_set && P_set) changed |= con.set(L, P, con(L, P) || ((con(L, K) && con(K, P)) || (con(L, M) && con(M, P)) || (con(L, N) && con(N, P)) || (con(L, O) && con(O, P)) || (con(L, T) && con(T, P)) || (con(L, U) && con(U, P)) || (con(L, V) && con(V, P)) || (con(L, W) && con(W, P))));
		if (L_set && Q_set) changed |= con.set(L, Q, con(L, Q) || ((con(L, K) && con(K, Q)) || (con(L, M) && con(M, Q)) || (con(L, N) && con(N, Q)) || (con(L, O) && con(O, Q)) || (con(L, P) && con(P, Q)) || (con(L, T) && con(T, Q)) || (con(L, U) && con(U, Q)) || (con(L, V) && con(V, Q)) || (con(L, W) && con(W, Q))));
		if (L_set && R_set) changed |= con.set(L, R, con(L, R) || ((con(L, K) && con(K, R)) || (con(L, M) && con(M, R)) || (con(L, N) && con(N, R)) || (con(L, O) && con(O, R)) || (con(L, P) && con(P, R)) || (con(L, T) && con(T, R)) || (con(L, U) && con(U, R)) || (con(L, V) && con(V, R)) || (con(L, W) && con(W, R))));
		if (L_set && S_set) changed |= con.set(L, S, con(L, S) || ((con(L, K) && con(K, S)) || (con(L, M) && con(M, S)) || (con(L, N) && con(N, S)) || (con(L, O) && con(O, S)) || (con(L, P) && con(P, S)) || (con(L, T) && con(T, S)) || (con(L, U) && con(U, S)) || (con(L, V) && con(V, S)) || (con(L, W) && con(W, S))));
		if (L_set && T_set) changed |= con.set(L, T, con(L, T) || ((con(L, K) && con(K, T)) || (con(L, M) && con(M, T)) || (con(L, N) && con(N, T)) || (con(L, O) && con(O, T)) || (con(L, P) && con(P, T)) || (con(L, U) && con(U, T)) || (con(L, V) && con(V, T)) || (con(L, W) && con(W, T))));
		if (L_set && U_set) changed |= con.set(L, U, con(L, U) || ((con(L, K) && con(K, U)) || (con(L, M) && con(M, U)) || (con(L, N) && con(N, U)) || (con(L, O) && con(O, U)) || (con(L, P) && con(P, U)) || (con(L, T) && con(T, U)) || (con(L, V) && con(V, U)) || (con(L, W) && con(W, U))));
		if (L_set && V_set) changed |= con.set(L, V, con(L, V) || ((con(L, K) && con(K, V)) || (con(L, M) && con(M, V)) || (con(L, N) && con(N, V)) || (con(L, O) && con(O, V)) || (con(L, P) && con(P, V)) || (con(L, T) && con(T, V)) || (con(L, U) && con(U, V)) || (con(L, W) && con(W, V))));
		if (L_set && W_set) changed |= con.set(L, W, con(L, W) || ((con(L, K) && con(K, W)) || (con(L, M) && con(M, W)) || (con(L, N) && con(N, W)) || (con(L, O) && con(O, W)) || (con(L, P) && con(P, W)) || (con(L, T) && con(T, W)) || (con(L, U) && con(U, W)) || (con(L, V) && con(V, W))));
		if (M_set && N_set) changed |= con.set(M, N, con(M, N) || ((con(M, L) && con(L, N)) || (con(M, O) && con(O, N)) || (con(M, P) && con(P, N)) || (con(M, U) && con(U, N)) || (con(M, V) && con(V, N))));
		if (M_set && O_set) changed |= con.set(M, O, con(M, O) || ((con(M, L) && con(L, O)) || (con(M, P) && con(P, O)) || (con(M, U) && con(U, O)) || (con(M, V) && con(V, O))));
		if (M_set && P_set) changed |= con.set(M, P, con(M, P) || ((con(M, L) && con(L, P)) || (con(M, O) && con(O, P)) || (con(M, U) && con(U, P)) || (con(M, V) && con(V, P))));
		if (M_set && Q_set) changed |= con.set(M, Q, con(M, Q) || ((con(M, L) && con(L, Q)) || (con(M, O) && con(O, Q)) || (con(M, P) && con(P, Q)) || (con(M, U) && con(U, Q)) || (con(M, V) && con(V, Q))));
		if (M_set && R_set) changed |= con.set(M, R, con(M, R) || ((con(M, L) && con(L, R)) || (con(M, O) && con(O, R)) || (con(M, P) && con(P, R)) || (con(M, U) && con(U, R)) || (con(M, V) && con(V, R))));
		if (M_set && S_set) changed |= con.set(M, S, con(M, S) || ((con(M, L) && con(L, S)) || (con(M, O) && con(O, S)) || (con(M, P) && con(P, S)) || (con(M, U) && con(U, S)) || (con(M, V) && con(V, S))));
		if (M_set && T_set) changed |= con.set(M, T, con(M, T) || ((con(M, L) && con(L, T)) || (con(M, O) && con(O, T)) || (con(M, P) && con(P, T)) || (con(M, U) && con(U, T)) || (con(M, V) && con(V, T))));
		if (M_set && U_set) changed |= con.set(M, U, con(M, U) || ((con(M, L) && con(L, U)) || (con(M, O) && con(O, U)) || (con(M, P) && con(P, U)) || (con(M, V) && con(V, U))));
		if (M_set && V_set) changed |= con.set(M, V, con(M, V) || ((con(M, L) && con(L, V)) || (con(M, O) && con(O, V)) || (con(M, P) && con(P, V)) || (con(M, U) && con(U, V))));
		if (M_set && W_set) changed |= con.set(M, W, con(M, W) || ((con(M, L) && con(L, W)) || (con(M, O) && con(O, W)) || (con(M, P) && con(P, W)) || (con(M, U) && con(U, W)) || (con(M, V) && con(V, W))));
		if (N_set && O_set) changed |= con.set(N, O, con(N, O) || ((con(N, K) && con(K, O)) || (con(N, L) && con(L, O)) || (con(N, Q) && con(Q, O)) || (con(N, R) && con(R, O)) || (con(N, T) && con(T, O)) || (con(N, U) && con(U, O)) || (con(N, W) && con(W, O))));
		if (N_set && P_set) changed |= con.set(N, P, con(N, P) || ((con(N, K) && con(K, P)) || (con(N, L) && con(L, P)) || (con(N, O) && con(O, P)) || (con(N, Q) && con(Q, P)) || (con(N, R) && con(R, P)) || (con(N, T) && con(T, P)) || (con(N, U) && con(U, P)) || (con(N, W) && con(W, P))));
		if (N_set && Q_set) changed |= con.set(N, Q, con(N, Q) || ((con(N, K) && con(K, Q)) || (con(N, L) && con(L, Q)) || (con(N, O) && con(O, Q)) || (con(N, R) && con(R, Q)) || (con(N, T) && con(T, Q)) || (con(N, U) && con(U, Q)) || (con(N, W) && con(W, Q))));
		if (N_set && R_set) changed |= con.set(N, R, con(N, R) || ((con(N, K) && con(K, R)) || (con(N, L) && con(L, R)) || (con(N, O) && con(O, R)) || (con(N, Q) && con(Q, R)) || (con(N, T) && con(T, R)) || (con(N, U) && con(U, R)) || (con(N, W) && con(W, R))));
		if (N_set && S_set) changed |= con.set(N, S, con(N, S) || ((con(N, K) && con(K, S)) || (con(N, L) && con(L, S)) || (con(N, O) && con(O, S)) || (con(N, Q) && con(Q, S)) || (con(N, R) && con(R, S)) || (con(N, T) && con(T, S)) || (con(N, U) && con(U, S)) || (con(N, W) && con(W, S))));
		if (N_set && T_set) changed |= con.set(N, T, con(N, T) || ((con(N, K) && con(K, T)) || (con(N, L) && con(L, T)) || (con(N, O) && con(O, T)) || (con(N, Q) && con(Q, T)) || (con(N, R) && con(R, T)) || (con(N, U) && con(U, T)) || (con(N, W) && con(W, T))));
		if (N_set && U_set) changed |= con.set(N, U, con(N, U) || ((con(N, K) && con(K, U)) || (con(N, L) && con(L, U)) || (con(N, O) && con(O, U)) || (con(N, Q) && con(Q, U)) || (con(N, R) && con(R, U)) || (con(N, T) && con(T, U)) || (con(N, W) && con(W, U))));
		if (N_set && V_set) changed |= con.set(N, V, con(N, V) || ((con(N, K) && con(K, V)) || (con(N, L) && con(L, V)) || (con(N, O) && con(O, V)) || (con(N, Q) && con(Q, V)) || (con(N, R) && con(R, V)) || (con(N, T) && con(T, V)) || (con(N, U) && con(U, V)) || (con(N, W) && con(W, V))));
		if (N_set && W_set) changed |= con.set(N, W, con(N, W) || ((con(N, K) && con(K, W)) || (con(N, L) && con(L, W)) || (con(N, O) && con(O, W)) || (con(N, Q) && con(Q, W)) || (con(N, R) && con(R, W)) || (con(N, T) && con(T, W)) || (con(N, U) && con(U, W))));
		if (O_set && P_set) changed |= con.set(O, P, con(O, P) || ((con(O, K) && con(K, P)) || (con(O, L) && con(L, P)) || (con(O, M) && con(M, P)) || (con(O, N) && con(N, P)) || (con(O, Q) && con(Q, P)) || (con(O, R) && con(R, P)) || (con(O, S) && con(S, P)) || (con(O, T) && con(T, P)) || (con(O, U) && con(U, P)) || (con(O, V) && con(V, P)) || (con(O, W) && con(W, P))));
		if (O_set && Q_set) changed |= con.set(O, Q, con(O, Q) || ((con(O, K) && con(K, Q)) || (con(O, L) && con(L, Q)) || (con(O, M) && con(M, Q)) || (con(O, N) && con(N, Q)) || (con(O, P) && con(P, Q)) || (con(O, R) && con(R, Q)) || (con(O, S) && con(S, Q)) || (con(O, T) && con(T, Q)) || (con(O, U) && con(U, Q)) || (con(O, V) && con(V, Q)) || (con(O, W) && con(W, Q))));
		if (O_set && R_set) changed |= con.set(O, R, con(O, R) || ((con(O, K) && con(K, R)) || (con(O, L) && con(L, R)) || (con(O, M) && con(M, R)) || (con(O, N) && con(N, R)) || (con(O, P) && con(P, R)) || (con(O, Q) && con(Q, R)) || (con(O, S) && con(S, R)) || (con(O, T) && con(T, R)) || (con(O, U) && con(U, R)) || (con(O, V) && con(V, R)) || (con(O, W) && con(W, R))));
		if (O_set && S_set) changed |= con.set(O, S, con(O, S) || ((con(O, K) && con(K, S)) || (con(O, L) && con(L, S)) || (con(O, M) && con(M, S)) || (con(O, N) && con(N, S)) || (con(O, P) && con(P, S)) || (con(O, Q) && con(Q, S)) || (con(O, R) && con(R, S)) || (con(O, T) && con(T, S)) || (con(O, U) && con(U, S)) || (con(O, V) && con(V, S)) || (con(O, W) && con(W, S))));
		if (O_set && T_set) changed |= con.set(O, T, con(O, T) || ((con(O, K) && con(K, T)) || (con(O, L) && con(L, T)) || (con(O, M) && con(M, T)) || (con(O, N) && con(N, T)) || (con(O, P) && con(P, T)) || (con(O, Q) && con(Q, T)) || (con(O, R) && con(R, T)) || (con(O, S) && con(S, T)) || (con(O, U) && con(U, T)) || (con(O, V) && con(V, T)) || (con(O, W) && con(W, T))));
		if (O_set && U_set) changed |= con.set(O, U, con(O, U) || ((con(O, K) && con(K, U)) || (con(O, L) && con(L, U)) || (con(O, M) && con(M, U)) || (con(O, N) && con(N, U)) || (con(O, P) && con(P, U)) || (con(O, Q) && con(Q, U)) || (con(O, R) && con(R, U)) || (con(O, S) && con(S, U)) || (con(O, T) && con(T, U)) || (con(O, V) && con(V, U)) || (con(O, W) && con(W, U))));
		if (O_set && V_set) changed |= con.set(O, V, con(O, V) || ((con(O, K) && con(K, V)) || (con(O, L) && con(L, V)) || (con(O, M) && con(M, V)) || (con(O, N) && con(N, V)) || (con(O, P) && con(P, V)) || (con(O, Q) && con(Q, V)) || (con(O, R) && con(R, V)) || (con(O, S) && con(S, V)) || (con(O, T) && con(T, V)) || (con(O, U) && con(U, V)) || (con(O, W) && con(W, V))));
		if (O_set && W_set) changed |= con.set(O, W, con(O, W) || ((con(O, K) && con(K, W)) || (con(O, L) && con(L, W)) || (con(O, M) && con(M, W)) || (con(O, N) && con(N, W)) || (con(O, P) && con(P, W)) || (con(O, Q) && con(Q, W)) || (con(O, R) && con(R, W)) || (con(O, S) && con(S, W)) || (con(O, T) && con(T, W)) || (con(O, U) && con(U, W)) || (con(O, V) && con(V, W))));
		if (P_set && Q_set) changed |= con.set(P, Q, con(P, Q) || ((con(P, L) && con(L, Q)) || (con(P, M) && con(M, Q)) || (con(P, O) && con(O, Q)) || (con(P, R) && con(R, Q)) || (con(P, S) && con(S, Q)) || (con(P, U) && con(U, Q)) || (con(P, V) && con(V, Q))));
		if (P_set && R_set) changed |= con.set(P, R, con(P, R) || ((con(P, L) && con(L, R)) || (con(P, M) && con(M, R)) || (con(P, O) && con(O, R)) || (con(P, S) && con(S, R)) || (con(P, U) && con(U, R)) || (con(P, V) && con(V, R))));
		if (P_set && S_set) changed |= con.set(P, S, con(P, S) || ((con(P, L) && con(L, S)) || (con(P, M) && con(M, S)) || (con(P, O) && con(O, S)) || (con(P, R) && con(R, S)) || (con(P, U) && con(U, S)) || (con(P, V) && con(V, S))));
		if (P_set && T_set) changed |= con.set(P, T, con(P, T) || ((con(P, L) && con(L, T)) || (con(P, M) && con(M, T)) || (con(P, O) && con(O, T)) || (con(P, R) && con(R, T)) || (con(P, S) && con(S, T)) || (con(P, U) && con(U, T)) || (con(P, V) && con(V, T))));
		if (P_set && U_set) changed |= con.set(P, U, con(P, U) || ((con(P, L) && con(L, U)) || (con(P, M) && con(M, U)) || (con(P, O) && con(O, U)) || (con(P, R) && con(R, U)) || (con(P, S) && con(S, U)) || (con(P, V) && con(V, U))));
		if (P_set && V_set) changed |= con.set(P, V, con(P, V) || ((con(P, L) && con(L, V)) || (con(P, M) && con(M, V)) || (con(P, O) && con(O, V)) || (con(P, R) && con(R, V)) || (con(P, S) && con(S, V)) || (con(P, U) && con(U, V))));
		if (P_set && W_set) changed |= con.set(P, W, con(P, W) || ((con(P, L) && con(L, W)) || (con(P, M) && con(M, W)) || (con(P, O) && con(O, W)) || (con(P, R) && con(R, W)) || (con(P, S) && con(S, W)) || (con(P, U) && con(U, W)) || (con(P, V) && con(V, W))));
		if (Q_set && R_set) changed |= con.set(Q, R, con(Q, R) || ((con(Q, N) && con(N, R)) || (con(Q, O) && con(O, R)) || (con(Q, W) && con(W, R))));
		if (Q_set && S_set) changed |= con.set(Q, S, con(Q, S) || ((con(Q, N) && con(N, S)) || (con(Q, O) && con(O, S)) || (con(Q, R) && con(R, S)) || (con(Q, W) && con(W, S))));
		if (Q_set && T_set) changed |= con.set(Q, T, con(Q, T) || ((con(Q, N) && con(N, T)) || (con(Q, O) && con(O, T)) || (con(Q, R) && con(R, T)) || (con(Q, W) && con(W, T))));
		if (Q_set && U_set) changed |= con.set(Q, U, con(Q, U) || ((con(Q, N) && con(N, U)) || (con(Q, O) && con(O, U)) || (con(Q, R) && con(R, U)) || (con(Q, W) && con(W, U))));
		if (Q_set && V_set) changed |= con.set(Q, V, con(Q, V) || ((con(Q, N) && con(N, V)) || (con(Q, O) && con(O, V)) || (con(Q, R) && con(R, V)) || (con(Q, W) && con(W, V))));
		if (Q_set && W_set) changed |= con.set(Q, W, con(Q, W) || ((con(Q, N) && con(N, W)) || (con(Q, O) && con(O, W)) || (con(Q, R) && con(R, W))));
		if (R_set && S_set) changed |= con.set(R, S, con(R, S) || ((con(R, N) && con(N, S)) || (con(R, O) && con(O, S)) || (con(R, P) && con(P, S)) || (con(R, Q) && con(Q, S)) || (con(R, W) && con(W, S))));
		if (R_set && T_set) changed |= con.set(R, T, con(R, T) || ((con(R, N) && con(N, T)) || (con(R, O) && con(O, T)) || (con(R, P) && con(P, T)) || (con(R, Q) && con(Q, T)) || (con(R, S) && con(S, T)) || (con(R, W) && con(W, T))));
		if (R_set && U_set) changed |= con.set(R, U, con(R, U) || ((con(R, N) && con(N, U)) || (con(R, O) && con(O, U)) || (con(R, P) && con(P, U)) || (con(R, Q) && con(Q, U)) || (con(R, S) && con(S, U)) || (con(R, W) && con(W, U))));
		if (R_set && V_set) changed |= con.set(R, V, con(R, V) || ((con(R, N) && con(N, V)) || (con(R, O) && con(O, V)) || (con(R, P) && con(P, V)) || (con(R, Q) && con(Q, V)) || (con(R, S) && con(S, V)) || (con(R, W) && con(W, V))));
		if (R_set && W_set) changed |= con.set(R, W, con(R, W) || ((con(R, N) && con(N, W)) || (con(R, O) && con(O, W)) || (con(R, P) && con(P, W)) || (con(R, Q) && con(Q, W)) || (con(R, S) && con(S, W))));
		if (S_set && T_set) changed |= con.set(S, T, con(S, T) || ((con(S, O) && con(O, T)) || (con(S, P) && con(P, T)) || (con(S, R) && con(R, T))));
		if (S_set && U_set) changed |= con.set(S, U, con(S, U) || ((con(S, O) && con(O, U)) || (con(S, P) && con(P, U)) || (con(S, R) && con(R, U))));
		if (S_set && V_set) changed |= con.set(S, V, con(S, V) || ((con(S, O) && con(O, V)) || (con(S, P) && con(P, V)) || (con(S, R) && con(R, V))));
		if (S_set && W_set) changed |= con.set(S, W, con(S, W) || ((con(S, O) && con(O, W)) || (con(S, P) && con(P, W)) || (con(S, R) && con(R, W))));
		if (T_set && U_set) changed |= con.set(T, U, con(T, U) || ((con(T, K) && con(K, U)) || (con(T, L) && con(L, U)) || (con(T, N) && con(N, U)) || (con(T, O) && con(O, U)) || (con(T, W) && con(W, U))));
		if (T_set && V_set) changed |= con.set(T, V, con(T, V) || ((con(T, K) && con(K, V)) || (con(T, L) && con(L, V)) || (con(T, N) && con(N, V)) || (con(T, O) && con(O, V)) || (con(T, U) && con(U, V)) || (con(T, W) && con(W, V))));
		if (T_set && W_set) changed |= con.set(T, W, con(T, W) || ((con(T, K) && con(K, W)) || (con(T, L) && con(L, W)) || (con(T, N) && con(N, W)) || (con(T, O) && con(O, W)) || (con(T, U) && con(U, W))));
		if (U_set && V_set) changed |= con.set(U, V, con(U, V) || ((con(U, K) && con(K, V)) || (con(U, L) && con(L, V)) || (con(U, M) && con(M, V)) || (con(U, N) && con(N, V)) || (con(U, O) && con(O, V)) || (con(U, P) && con(P, V)) || (con(U, T) && con(T, V)) || (con(U, W) && con(W, V))));
		if (U_set && W_set) changed |= con.set(U, W, con(U, W) || ((con(U, K) && con(K, W)) || (con(U, L) && con(L, W)) || (con(U, M) && con(M, W)) || (con(U, N) && con(N, W)) || (con(U, O) && con(O, W)) || (con(U, P) && con(P, W)) || (con(U, T) && con(T, W)) || (con(U, V) && con(V, W))));
		if (V_set && W_set) changed |= con.set(V, W, con(V, W) || ((con(V, L) && con(L, W)) || (con(V, M) && con(M, W)) || (con(V, O) && con(O, W)) || (con(V, P) && con(P, W)) || (con(V, U) && con(U, W))));
	}
	MergeSet ms(con);
	ms.BuildMergeSet();

	action_bitset combined_actions(ms.mergesets_.size());
	for (const auto& s : ms.mergesets_) {
		std::string action = "x<-";
		if (s.empty())
			action += "newlabel";
		else {
			action += s[0];
			for (size_t i = 1; i < s.size(); ++i)
				action += "+" + s[i];
		}
		combined_actions.set(rs.actions_pos.at(action) - 1);
	}
	return combined_actions;
}


class Grana3dRS : public BaseRuleSet {

public:

	using BaseRuleSet::BaseRuleSet;

	rule_set GenerateRuleSet()
	{

		pixel_set grana_mask{
			// Regex for changing blacklist: >  ({ ".{2}", {.{2},.{2},.{2}} },)  <

			// First Plane, first row
			/*{ "Ka", {-2,-2,-2} },{ "Kb", {-1,-2,-2} },{ "Kc", {-2,-1,-2} },{ "Kd", {-1,-1,-2} },
			{ "Ke", {-2,-2,-1} },{ "Kf", {-1,-2,-1} },{ "Kg", {-2,-1,-1} },*/{ "Kh", {-1,-1,-1} },

			/*{ "La", {+0,-2,-2} },{ "Lb", {+1,-2,-2} },{ "Lc", {+0,-1,-2} },{ "Ld", {+1,-1,-2} },
			{ "Le", {+0,-2,-1} },{ "Lf", {+1,-2,-1} },*/{ "Lg", {+0,-1,-1} },{ "Lh", {+1,-1,-1} },

			/*{ "Ma", {+2,-2,-2} },{ "Mb", {+3,-2,-2} },{ "Mc", {+2,-1,-2} },{ "Md", {+3,-1,-2} },
			{ "Me", {+2,-2,-1} },{ "Mf", {+3,-2,-1} },*/{ "Mg", {+2,-1,-1} },/*{ "Mh", {+3,-1,-1} },*/

			// First plane, second row																	    
			/*{ "Na", {-2,+0,-2} },{ "Nb", {-1,+0,-2} },{ "Nc", {-2,+1,-2} },{ "Nd", {-1,+1,-2} },
			{ "Ne", {-2,+0,-1} },*/{ "Nf", {-1,+0,-1} },/*{ "Ng", {-2,+1,-1} },*/{ "Nh", {-1,+1,-1} },

			/*{ "Oa", {+0,+0,-2} },{ "Ob", {+1,+0,-2} },{ "Oc", {+0,+1,-2} },{ "Od", {+1,+1,-2} },*/
			{ "Oe", {+0,+0,-1} },{ "Of", {+1,+0,-1} },{ "Og", {+0,+1,-1} },{ "Oh", {+1,+1,-1} },

			/*{ "Pa", {+2,+0,-2} },{ "Pb", {+3,+0,-2} },{ "Pc", {+2,+1,-2} },{ "Pd", {+3,+1,-2} },*/
			{ "Pe", {+2,+0,-1} },/*{ "Pf", {+3,+0,-1} },*/{ "Pg", {+2,+1,-1} },/*{ "Ph", {+3,+1,-1} },*/

			// First plane, third row																		    
			/*{ "Qa", {-2,+2,-2} },{ "Qb", {-1,+2,-2} },{ "Qc", {-2,+3,-2} },{ "Qd", {-1,+3,-2} },
			{ "Qe", {-2,+2,-1} },*/{ "Qf", {-1,+2,-1} },/*{ "Qg", {-2,+3,-1} },{ "Qh", {-1,+3,-1} },*/

			/*{ "Ra", {+0,+2,-2} },{ "Rb", {+1,+2,-2} },{ "Rc", {+0,+3,-2} },{ "Rd", {+1,+3,-2} },*/
			{ "Re", {+0,+2,-1} },{ "Rf", {+1,+2,-1} },/*{ "Rg", {+0,+3,-1} },{ "Rh", {+1,+3,-1} },*/

			/*{ "Sa", {+2,+2,-2} },{ "Sb", {+3,+2,-2} },{ "Sc", {+2,+3,-2} },{ "Sd", {+3,+3,-2} },*/
			{ "Se", {+2,+2,-1} },/*{ "Sf", {+3,+2,-1} },{ "Sg", {+2,+3,-1} },{ "Sh", {+3,+3,-1} },*/

			// Second Plane, first row																	    
			/*{ "Ta", {-2,-2,+0} },{ "Tb", {-1,-2,+0} },{ "Tc", {-2,-1,+0} },*/{ "Td", {-1,-1,+0} },
			/*{ "Te", {-2,-2,+1} },{ "Tf", {-1,-2,+1} },{ "Tg", {-2,-1,+1} },*/{ "Th", {-1,-1,+1} },

			/*{ "Ua", {+0,-2,+0} },{ "Ub", {+1,-2,+0} },*/{ "Uc", {+0,-1,+0} },{ "Ud", {+1,-1,+0} },
			/*{ "Ue", {+0,-2,+1} },{ "Uf", {+1,-2,+1} },*/{ "Ug", {+0,-1,+1} },{ "Uh", {+1,-1,+1} },

			/*{ "Va", {+2,-2,+0} },{ "Vb", {+3,-2,+0} },*/{ "Vc", {+2,-1,+0} },/*{ "Vd", {+3,-1,+0} },*/
			/*{ "Ve", {+2,-2,+1} },{ "Vf", {+3,-2,+1} },*/{ "Vg", {+2,-1,+1} },/*{ "Vh", {+3,-1,+1} },*/

			// Second plane, second row																	    
			/*{ "Wa", {-2,+0,+0} },*/{ "Wb", {-1,+0,+0} },/*{ "Wc", {-2,+1,+0} },*/{ "Wd", {-1,+1,+0} },
			/*{ "We", {-2,+0,+1} },*/{ "Wf", {-1,+0,+1} },/*{ "Wg", {-2,+1,+1} },*/{ "Wh", {-1,+1,+1} },

			{ "Xa", {+0,+0,+0} },{ "Xb", {+1,+0,+0} },{ "Xc", {+0,+1,+0} },{ "Xd", {+1,+1,+0} },
			{ "Xe", {+0,+0,+1} },{ "Xf", {+1,+0,+1} },{ "Xg", {+0,+1,+1} },{ "Xh", {+1,+1,+1} },

		};
		grana_mask.SetShifts({ 2, 2, 2 });

		rule_set labeling;
		labeling.InitConditions(grana_mask);
		labeling.InitActions({ 
			// reduced: 2829 actions
				"nothing", "x<-newlabel", "x<-K", "x<-L", "x<-M", "x<-N", "x<-O", "x<-P", "x<-Q", "x<-R", "x<-S", "x<-T", "x<-U", "x<-V", "x<-W", "x<-K+L", "x<-K+M", "x<-K+N", "x<-K+O", "x<-K+P", "x<-K+Q", "x<-K+R", "x<-K+S", "x<-K+T", "x<-K+U", "x<-K+V", "x<-K+W", "x<-L+M", "x<-L+N", "x<-L+O", "x<-L+P", "x<-L+Q", "x<-L+R", "x<-L+S", "x<-L+T", "x<-L+U", "x<-L+V", "x<-L+W", "x<-M+N", "x<-M+O", "x<-M+P", "x<-M+Q", "x<-M+R", "x<-M+S", "x<-M+T", "x<-M+U", "x<-M+V", "x<-M+W", "x<-N+O", "x<-N+P", "x<-N+Q", "x<-N+R", "x<-N+S", "x<-N+T", "x<-N+U", "x<-N+V", "x<-N+W", "x<-O+P", "x<-O+Q", "x<-O+R", "x<-O+S", "x<-O+T", "x<-O+U", "x<-O+V", "x<-O+W", "x<-P+Q", "x<-P+R", "x<-P+S", "x<-P+T", "x<-P+U", "x<-P+V", "x<-P+W", "x<-Q+R", "x<-Q+S", "x<-Q+T", "x<-Q+U", "x<-Q+V", "x<-Q+W", "x<-R+S", "x<-R+T", "x<-R+U", "x<-R+V", "x<-R+W", "x<-S+T", "x<-S+U", "x<-S+V", "x<-S+W", "x<-T+U", "x<-T+V", "x<-T+W", "x<-U+V", "x<-U+W", "x<-V+W", "x<-K+L+N", "x<-K+L+O", "x<-K+L+P", "x<-K+L+Q", "x<-K+L+R", "x<-K+L+S", "x<-K+L+T", "x<-K+L+U", "x<-K+L+V", "x<-K+L+W", "x<-K+M+N", "x<-K+M+O", "x<-K+M+P", "x<-K+M+Q", "x<-K+M+R", "x<-K+M+S", "x<-K+M+T", "x<-K+M+U", "x<-K+M+V", "x<-K+M+W", "x<-K+N+O", "x<-K+N+P", "x<-K+N+R", "x<-K+N+S", "x<-K+N+T", "x<-K+N+U", "x<-K+N+V", "x<-K+N+W", "x<-K+O+P", "x<-K+O+Q", "x<-K+O+R", "x<-K+O+S", "x<-K+O+T", "x<-K+O+U", "x<-K+O+V", "x<-K+O+W", "x<-K+P+Q", "x<-K+P+R", "x<-K+P+S", "x<-K+P+T", "x<-K+P+U", "x<-K+P+V", "x<-K+P+W", "x<-K+Q+R", "x<-K+Q+S", "x<-K+Q+T", "x<-K+Q+U", "x<-K+Q+V", "x<-K+Q+W", "x<-K+R+S", "x<-K+R+T", "x<-K+R+U", "x<-K+R+V", "x<-K+R+W", "x<-K+S+T", "x<-K+S+U", "x<-K+S+V", "x<-K+S+W", "x<-K+T+U", "x<-K+T+V", "x<-K+T+W", "x<-K+U+V", "x<-K+U+W", "x<-K+V+W", "x<-L+M+N", "x<-L+M+O", "x<-L+M+P", "x<-L+M+Q", "x<-L+M+R", "x<-L+M+S", "x<-L+M+T", "x<-L+M+U", "x<-L+M+V", "x<-L+M+W", "x<-L+N+O", "x<-L+N+P", "x<-L+N+Q", "x<-L+N+R", "x<-L+N+S", "x<-L+N+T", "x<-L+N+U", "x<-L+N+V", "x<-L+N+W", "x<-L+O+P", "x<-L+O+Q", "x<-L+O+S", "x<-L+O+T", "x<-L+O+U", "x<-L+O+V", "x<-L+O+W", "x<-L+P+Q", "x<-L+P+R", "x<-L+P+S", "x<-L+P+T", "x<-L+P+U", "x<-L+P+V", "x<-L+P+W", "x<-L+Q+R", "x<-L+Q+S", "x<-L+Q+T", "x<-L+Q+U", "x<-L+Q+V", "x<-L+Q+W", "x<-L+R+S", "x<-L+R+T", "x<-L+R+U", "x<-L+R+V", "x<-L+R+W", "x<-L+S+T", "x<-L+S+U", "x<-L+S+V", "x<-L+S+W", "x<-L+T+U", "x<-L+T+V", "x<-L+T+W", "x<-L+U+V", "x<-L+U+W", "x<-L+V+W", "x<-M+N+O", "x<-M+N+P", "x<-M+N+Q", "x<-M+N+R", "x<-M+N+S", "x<-M+N+T", "x<-M+N+U", "x<-M+N+V", "x<-M+N+W", "x<-M+O+P", "x<-M+O+Q", "x<-M+O+R", "x<-M+O+S", "x<-M+O+T", "x<-M+O+U", "x<-M+O+V", "x<-M+O+W", "x<-M+P+Q", "x<-M+P+R", "x<-M+P+T", "x<-M+P+U", "x<-M+P+V", "x<-M+P+W", "x<-M+Q+R", "x<-M+Q+S", "x<-M+Q+T", "x<-M+Q+U", "x<-M+Q+V", "x<-M+Q+W", "x<-M+R+S", "x<-M+R+T", "x<-M+R+U", "x<-M+R+V", "x<-M+R+W", "x<-M+S+T", "x<-M+S+U", "x<-M+S+V", "x<-M+S+W", "x<-M+T+U", "x<-M+T+V", "x<-M+T+W", "x<-M+U+V", "x<-M+U+W", "x<-M+V+W", "x<-N+O+Q", "x<-N+O+R", "x<-N+O+S", "x<-N+O+T", "x<-N+O+U", "x<-N+O+V", "x<-N+O+W", "x<-N+P+Q", "x<-N+P+R", "x<-N+P+S", "x<-N+P+T", "x<-N+P+U", "x<-N+P+V", "x<-N+P+W", "x<-N+Q+R", "x<-N+Q+S", "x<-N+Q+T", "x<-N+Q+U", "x<-N+Q+V", "x<-N+Q+W", "x<-N+R+S", "x<-N+R+T", "x<-N+R+U", "x<-N+R+V", "x<-N+R+W", "x<-N+S+T", "x<-N+S+U", "x<-N+S+V", "x<-N+S+W", "x<-N+T+U", "x<-N+T+V", "x<-N+T+W", "x<-N+U+V", "x<-N+U+W", "x<-N+V+W", "x<-O+P+Q", "x<-O+P+R", "x<-O+P+S", "x<-O+P+T", "x<-O+P+U", "x<-O+P+V", "x<-O+P+W", "x<-O+Q+R", "x<-O+Q+S", "x<-O+Q+T", "x<-O+Q+U", "x<-O+Q+V", "x<-O+Q+W", "x<-O+R+S", "x<-O+R+T", "x<-O+R+U", "x<-O+R+V", "x<-O+R+W", "x<-O+S+T", "x<-O+S+U", "x<-O+S+V", "x<-O+S+W", "x<-O+T+U", "x<-O+T+V", "x<-O+T+W", "x<-O+U+V", "x<-O+U+W", "x<-O+V+W", "x<-P+Q+R", "x<-P+Q+S", "x<-P+Q+T", "x<-P+Q+U", "x<-P+Q+V", "x<-P+Q+W", "x<-P+R+S", "x<-P+R+T", "x<-P+R+U", "x<-P+R+V", "x<-P+R+W", "x<-P+S+T", "x<-P+S+U", "x<-P+S+V", "x<-P+S+W", "x<-P+T+U", "x<-P+T+V", "x<-P+T+W", "x<-P+U+V", "x<-P+U+W", "x<-P+V+W", "x<-Q+R+T", "x<-Q+R+U", "x<-Q+R+V", "x<-Q+R+W", "x<-Q+S+T", "x<-Q+S+U", "x<-Q+S+V", "x<-Q+S+W", "x<-Q+T+U", "x<-Q+T+V", "x<-Q+T+W", "x<-Q+U+V", "x<-Q+U+W", "x<-Q+V+W", "x<-R+S+T", "x<-R+S+U", "x<-R+S+V", "x<-R+S+W", "x<-R+T+U", "x<-R+T+V", "x<-R+T+W", "x<-R+U+V", "x<-R+U+W", "x<-R+V+W", "x<-S+T+U", "x<-S+T+V", "x<-S+T+W", "x<-S+U+V", "x<-S+U+W", "x<-S+V+W", "x<-T+U+W", "x<-T+V+W", "x<-U+V+W", "x<-K+L+N+O", "x<-K+L+N+P", "x<-K+L+N+R", "x<-K+L+N+S", "x<-K+L+N+T", "x<-K+L+N+U", "x<-K+L+N+V", "x<-K+L+N+W", "x<-K+L+O+P", "x<-K+L+O+Q", "x<-K+L+O+S", "x<-K+L+O+T", "x<-K+L+O+U", "x<-K+L+O+V", "x<-K+L+O+W", "x<-K+L+P+Q", "x<-K+L+P+R", "x<-K+L+P+T", "x<-K+L+P+U", "x<-K+L+P+V", "x<-K+L+P+W", "x<-K+L+Q+R", "x<-K+L+Q+S", "x<-K+L+Q+T", "x<-K+L+Q+U", "x<-K+L+Q+V", "x<-K+L+Q+W", "x<-K+L+R+S", "x<-K+L+R+T", "x<-K+L+R+U", "x<-K+L+R+V", "x<-K+L+R+W", "x<-K+L+S+T", "x<-K+L+S+U", "x<-K+L+S+V", "x<-K+L+S+W", "x<-K+L+T+U", "x<-K+L+T+V", "x<-K+L+T+W", "x<-K+L+U+V", "x<-K+L+U+W", "x<-K+L+V+W", "x<-K+M+N+O", "x<-K+M+N+P", "x<-K+M+N+R", "x<-K+M+N+S", "x<-K+M+N+T", "x<-K+M+N+U", "x<-K+M+N+V", "x<-K+M+N+W", "x<-K+M+O+P", "x<-K+M+O+Q", "x<-K+M+O+S", "x<-K+M+O+T", "x<-K+M+O+U", "x<-K+M+O+V", "x<-K+M+O+W", "x<-K+M+P+Q", "x<-K+M+P+R", "x<-K+M+P+T", "x<-K+M+P+U", "x<-K+M+P+V", "x<-K+M+P+W", "x<-K+M+Q+R", "x<-K+M+Q+S", "x<-K+M+Q+T", "x<-K+M+Q+U", "x<-K+M+Q+V", "x<-K+M+Q+W", "x<-K+M+R+S", "x<-K+M+R+T", "x<-K+M+R+U", "x<-K+M+R+V", "x<-K+M+R+W", "x<-K+M+S+T", "x<-K+M+S+U", "x<-K+M+S+V", "x<-K+M+S+W", "x<-K+M+T+U", "x<-K+M+T+V", "x<-K+M+T+W", "x<-K+M+U+V", "x<-K+M+U+W", "x<-K+M+V+W", "x<-K+N+O+R", "x<-K+N+O+S", "x<-K+N+O+T", "x<-K+N+O+U", "x<-K+N+O+V", "x<-K+N+O+W", "x<-K+N+P+R", "x<-K+N+P+S", "x<-K+N+P+T", "x<-K+N+P+U", "x<-K+N+P+V", "x<-K+N+P+W", "x<-K+N+R+T", "x<-K+N+R+U", "x<-K+N+R+V", "x<-K+N+R+W", "x<-K+N+S+T", "x<-K+N+S+U", "x<-K+N+S+V", "x<-K+N+S+W", "x<-K+N+T+U", "x<-K+N+T+V", "x<-K+N+T+W", "x<-K+N+U+V", "x<-K+N+U+W", "x<-K+N+V+W", "x<-K+O+P+S", "x<-K+O+P+T", "x<-K+O+P+U", "x<-K+O+P+V", "x<-K+O+P+W", "x<-K+O+Q+R", "x<-K+O+Q+S", "x<-K+O+Q+T", "x<-K+O+Q+U", "x<-K+O+Q+V", "x<-K+O+Q+W", "x<-K+O+R+S", "x<-K+O+R+T", "x<-K+O+R+U", "x<-K+O+R+V", "x<-K+O+R+W", "x<-K+O+S+T", "x<-K+O+S+U", "x<-K+O+S+V", "x<-K+O+S+W", "x<-K+O+T+U", "x<-K+O+T+V", "x<-K+O+T+W", "x<-K+O+U+V", "x<-K+O+U+W", "x<-K+O+V+W", "x<-K+P+Q+R", "x<-K+P+Q+S", "x<-K+P+Q+T", "x<-K+P+Q+U", "x<-K+P+Q+V", "x<-K+P+Q+W", "x<-K+P+R+S", "x<-K+P+R+T", "x<-K+P+R+U", "x<-K+P+R+V", "x<-K+P+R+W", "x<-K+P+S+T", "x<-K+P+S+U", "x<-K+P+S+V", "x<-K+P+S+W", "x<-K+P+T+U", "x<-K+P+T+V", "x<-K+P+T+W", "x<-K+P+U+V", "x<-K+P+U+W", "x<-K+P+V+W", "x<-K+Q+R+T", "x<-K+Q+R+U", "x<-K+Q+R+V", "x<-K+Q+R+W", "x<-K+Q+S+T", "x<-K+Q+S+U", "x<-K+Q+S+V", "x<-K+Q+S+W", "x<-K+Q+T+U", "x<-K+Q+T+V", "x<-K+Q+T+W", "x<-K+Q+U+V", "x<-K+Q+U+W", "x<-K+Q+V+W", "x<-K+R+S+T", "x<-K+R+S+U", "x<-K+R+S+V", "x<-K+R+S+W", "x<-K+R+T+U", "x<-K+R+T+V", "x<-K+R+T+W", "x<-K+R+U+V", "x<-K+R+U+W", "x<-K+R+V+W", "x<-K+S+T+U", "x<-K+S+T+V", "x<-K+S+T+W", "x<-K+S+U+V", "x<-K+S+U+W", "x<-K+S+V+W", "x<-K+T+U+W", "x<-K+T+V+W", "x<-K+U+V+W", "x<-L+M+N+O", "x<-L+M+N+P", "x<-L+M+N+R", "x<-L+M+N+S", "x<-L+M+N+T", "x<-L+M+N+U", "x<-L+M+N+V", "x<-L+M+N+W", "x<-L+M+O+P", "x<-L+M+O+Q", "x<-L+M+O+S", "x<-L+M+O+T", "x<-L+M+O+U", "x<-L+M+O+V", "x<-L+M+O+W", "x<-L+M+P+Q", "x<-L+M+P+R", "x<-L+M+P+T", "x<-L+M+P+U", "x<-L+M+P+V", "x<-L+M+P+W", "x<-L+M+Q+R", "x<-L+M+Q+S", "x<-L+M+Q+T", "x<-L+M+Q+U", "x<-L+M+Q+V", "x<-L+M+Q+W", "x<-L+M+R+S", "x<-L+M+R+T", "x<-L+M+R+U", "x<-L+M+R+V", "x<-L+M+R+W", "x<-L+M+S+T", "x<-L+M+S+U", "x<-L+M+S+V", "x<-L+M+S+W", "x<-L+M+T+U", "x<-L+M+T+V", "x<-L+M+T+W", "x<-L+M+U+V", "x<-L+M+U+W", "x<-L+M+V+W", "x<-L+N+O+Q", "x<-L+N+O+T", "x<-L+N+O+U", "x<-L+N+O+V", "x<-L+N+O+W", "x<-L+N+P+Q", "x<-L+N+P+R", "x<-L+N+P+S", "x<-L+N+P+T", "x<-L+N+P+U", "x<-L+N+P+V", "x<-L+N+P+W", "x<-L+N+Q+R", "x<-L+N+Q+S", "x<-L+N+Q+T", "x<-L+N+Q+U", "x<-L+N+Q+V", "x<-L+N+Q+W", "x<-L+N+R+S", "x<-L+N+R+T", "x<-L+N+R+U", "x<-L+N+R+V", "x<-L+N+R+W", "x<-L+N+S+T", "x<-L+N+S+U", "x<-L+N+S+V", "x<-L+N+S+W", "x<-L+N+T+U", "x<-L+N+T+V", "x<-L+N+T+W", "x<-L+N+U+V", "x<-L+N+U+W", "x<-L+N+V+W", "x<-L+O+P+S", "x<-L+O+P+T", "x<-L+O+P+U", "x<-L+O+P+V", "x<-L+O+P+W", "x<-L+O+Q+T", "x<-L+O+Q+U", "x<-L+O+Q+V", "x<-L+O+Q+W", "x<-L+O+S+T", "x<-L+O+S+U", "x<-L+O+S+V", "x<-L+O+S+W", "x<-L+O+T+U", "x<-L+O+T+V", "x<-L+O+T+W", "x<-L+O+U+V", "x<-L+O+U+W", "x<-L+O+V+W", "x<-L+P+Q+R", "x<-L+P+Q+S", "x<-L+P+Q+T", "x<-L+P+Q+U", "x<-L+P+Q+V", "x<-L+P+Q+W", "x<-L+P+R+S", "x<-L+P+R+T", "x<-L+P+R+U", "x<-L+P+R+V", "x<-L+P+R+W", "x<-L+P+S+T", "x<-L+P+S+U", "x<-L+P+S+V", "x<-L+P+S+W", "x<-L+P+T+U", "x<-L+P+T+V", "x<-L+P+T+W", "x<-L+P+U+V", "x<-L+P+U+W", "x<-L+P+V+W", "x<-L+Q+R+T", "x<-L+Q+R+U", "x<-L+Q+R+V", "x<-L+Q+R+W", "x<-L+Q+S+T", "x<-L+Q+S+U", "x<-L+Q+S+V", "x<-L+Q+S+W", "x<-L+Q+T+U", "x<-L+Q+T+V", "x<-L+Q+T+W", "x<-L+Q+U+V", "x<-L+Q+U+W", "x<-L+Q+V+W", "x<-L+R+S+T", "x<-L+R+S+U", "x<-L+R+S+V", "x<-L+R+S+W", "x<-L+R+T+U", "x<-L+R+T+V", "x<-L+R+T+W", "x<-L+R+U+V", "x<-L+R+U+W", "x<-L+R+V+W", "x<-L+S+T+U", "x<-L+S+T+V", "x<-L+S+T+W", "x<-L+S+U+V", "x<-L+S+U+W", "x<-L+S+V+W", "x<-L+T+U+W", "x<-L+T+V+W", "x<-L+U+V+W", "x<-M+N+O+Q", "x<-M+N+O+T", "x<-M+N+O+U", "x<-M+N+O+V", "x<-M+N+O+W", "x<-M+N+P+Q", "x<-M+N+P+R", "x<-M+N+P+T", "x<-M+N+P+U", "x<-M+N+P+V", "x<-M+N+P+W", "x<-M+N+Q+R", "x<-M+N+Q+S", "x<-M+N+Q+T", "x<-M+N+Q+U", "x<-M+N+Q+V", "x<-M+N+Q+W", "x<-M+N+R+S", "x<-M+N+R+T", "x<-M+N+R+U", "x<-M+N+R+V", "x<-M+N+R+W", "x<-M+N+S+T", "x<-M+N+S+U", "x<-M+N+S+V", "x<-M+N+S+W", "x<-M+N+T+U", "x<-M+N+T+V", "x<-M+N+T+W", "x<-M+N+U+V", "x<-M+N+U+W", "x<-M+N+V+W", "x<-M+O+P+Q", "x<-M+O+P+R", "x<-M+O+P+T", "x<-M+O+P+U", "x<-M+O+P+V", "x<-M+O+P+W", "x<-M+O+Q+R", "x<-M+O+Q+S", "x<-M+O+Q+T", "x<-M+O+Q+U", "x<-M+O+Q+V", "x<-M+O+Q+W", "x<-M+O+R+S", "x<-M+O+R+T", "x<-M+O+R+U", "x<-M+O+R+V", "x<-M+O+R+W", "x<-M+O+S+T", "x<-M+O+S+U", "x<-M+O+S+V", "x<-M+O+S+W", "x<-M+O+T+U", "x<-M+O+T+V", "x<-M+O+T+W", "x<-M+O+U+V", "x<-M+O+U+W", "x<-M+O+V+W", "x<-M+P+Q+T", "x<-M+P+Q+U", "x<-M+P+Q+V", "x<-M+P+Q+W", "x<-M+P+R+T", "x<-M+P+R+U", "x<-M+P+R+V", "x<-M+P+R+W", "x<-M+P+T+U", "x<-M+P+T+V", "x<-M+P+T+W", "x<-M+P+U+V", "x<-M+P+U+W", "x<-M+P+V+W", "x<-M+Q+R+T", "x<-M+Q+R+U", "x<-M+Q+R+V", "x<-M+Q+R+W", "x<-M+Q+S+T", "x<-M+Q+S+U", "x<-M+Q+S+V", "x<-M+Q+S+W", "x<-M+Q+T+U", "x<-M+Q+T+V", "x<-M+Q+T+W", "x<-M+Q+U+V", "x<-M+Q+U+W", "x<-M+Q+V+W", "x<-M+R+S+T", "x<-M+R+S+U", "x<-M+R+S+V", "x<-M+R+S+W", "x<-M+R+T+U", "x<-M+R+T+V", "x<-M+R+T+W", "x<-M+R+U+V", "x<-M+R+U+W", "x<-M+R+V+W", "x<-M+S+T+U", "x<-M+S+T+V", "x<-M+S+T+W", "x<-M+S+U+V", "x<-M+S+U+W", "x<-M+S+V+W", "x<-M+T+U+W", "x<-M+T+V+W", "x<-M+U+V+W", "x<-N+O+Q+R", "x<-N+O+Q+S", "x<-N+O+Q+T", "x<-N+O+Q+U", "x<-N+O+Q+V", "x<-N+O+Q+W", "x<-N+O+R+S", "x<-N+O+R+T", "x<-N+O+R+U", "x<-N+O+R+V", "x<-N+O+R+W", "x<-N+O+S+T", "x<-N+O+S+U", "x<-N+O+S+V", "x<-N+O+S+W", "x<-N+O+T+U", "x<-N+O+T+V", "x<-N+O+T+W", "x<-N+O+U+V", "x<-N+O+U+W", "x<-N+O+V+W", "x<-N+P+Q+R", "x<-N+P+Q+S", "x<-N+P+Q+T", "x<-N+P+Q+U", "x<-N+P+Q+V", "x<-N+P+Q+W", "x<-N+P+R+S", "x<-N+P+R+T", "x<-N+P+R+U", "x<-N+P+R+V", "x<-N+P+R+W", "x<-N+P+S+T", "x<-N+P+S+U", "x<-N+P+S+V", "x<-N+P+S+W", "x<-N+P+T+U", "x<-N+P+T+V", "x<-N+P+T+W", "x<-N+P+U+V", "x<-N+P+U+W", "x<-N+P+V+W", "x<-N+Q+R+T", "x<-N+Q+R+U", "x<-N+Q+R+V", "x<-N+Q+R+W", "x<-N+Q+S+T", "x<-N+Q+S+U", "x<-N+Q+S+V", "x<-N+Q+S+W", "x<-N+Q+T+U", "x<-N+Q+T+V", "x<-N+Q+T+W", "x<-N+Q+U+V", "x<-N+Q+U+W", "x<-N+Q+V+W", "x<-N+R+S+T", "x<-N+R+S+U", "x<-N+R+S+V", "x<-N+R+S+W", "x<-N+R+T+U", "x<-N+R+T+V", "x<-N+R+T+W", "x<-N+R+U+V", "x<-N+R+U+W", "x<-N+R+V+W", "x<-N+S+T+U", "x<-N+S+T+V", "x<-N+S+T+W", "x<-N+S+U+V", "x<-N+S+U+W", "x<-N+S+V+W", "x<-N+T+U+W", "x<-N+T+V+W", "x<-N+U+V+W", "x<-O+P+Q+R", "x<-O+P+Q+S", "x<-O+P+Q+T", "x<-O+P+Q+U", "x<-O+P+Q+V", "x<-O+P+Q+W", "x<-O+P+R+S", "x<-O+P+R+T", "x<-O+P+R+U", "x<-O+P+R+V", "x<-O+P+R+W", "x<-O+P+S+T", "x<-O+P+S+U", "x<-O+P+S+V", "x<-O+P+S+W", "x<-O+P+T+U", "x<-O+P+T+V", "x<-O+P+T+W", "x<-O+P+U+V", "x<-O+P+U+W", "x<-O+P+V+W", "x<-O+Q+R+T", "x<-O+Q+R+U", "x<-O+Q+R+V", "x<-O+Q+R+W", "x<-O+Q+S+T", "x<-O+Q+S+U", "x<-O+Q+S+V", "x<-O+Q+S+W", "x<-O+Q+T+U", "x<-O+Q+T+V", "x<-O+Q+T+W", "x<-O+Q+U+V", "x<-O+Q+U+W", "x<-O+Q+V+W", "x<-O+R+S+T", "x<-O+R+S+U", "x<-O+R+S+V", "x<-O+R+S+W", "x<-O+R+T+U", "x<-O+R+T+V", "x<-O+R+T+W", "x<-O+R+U+V", "x<-O+R+U+W", "x<-O+R+V+W", "x<-O+S+T+U", "x<-O+S+T+V", "x<-O+S+T+W", "x<-O+S+U+V", "x<-O+S+U+W", "x<-O+S+V+W", "x<-O+T+U+W", "x<-O+T+V+W", "x<-O+U+V+W", "x<-P+Q+R+T", "x<-P+Q+R+U", "x<-P+Q+R+V", "x<-P+Q+R+W", "x<-P+Q+S+T", "x<-P+Q+S+U", "x<-P+Q+S+V", "x<-P+Q+S+W", "x<-P+Q+T+U", "x<-P+Q+T+V", "x<-P+Q+T+W", "x<-P+Q+U+V", "x<-P+Q+U+W", "x<-P+Q+V+W", "x<-P+R+S+T", "x<-P+R+S+U", "x<-P+R+S+V", "x<-P+R+S+W", "x<-P+R+T+U", "x<-P+R+T+V", "x<-P+R+T+W", "x<-P+R+U+V", "x<-P+R+U+W", "x<-P+R+V+W", "x<-P+S+T+U", "x<-P+S+T+V", "x<-P+S+T+W", "x<-P+S+U+V", "x<-P+S+U+W", "x<-P+S+V+W", "x<-P+T+U+W", "x<-P+T+V+W", "x<-P+U+V+W", "x<-Q+R+T+U", "x<-Q+R+T+V", "x<-Q+R+T+W", "x<-Q+R+U+V", "x<-Q+R+U+W", "x<-Q+R+V+W", "x<-Q+S+T+U", "x<-Q+S+T+V", "x<-Q+S+T+W", "x<-Q+S+U+V", "x<-Q+S+U+W", "x<-Q+S+V+W", "x<-Q+T+U+W", "x<-Q+T+V+W", "x<-Q+U+V+W", "x<-R+S+T+U", "x<-R+S+T+V", "x<-R+S+T+W", "x<-R+S+U+V", "x<-R+S+U+W", "x<-R+S+V+W", "x<-R+T+U+W", "x<-R+T+V+W", "x<-R+U+V+W", "x<-S+T+U+W", "x<-S+T+V+W", "x<-S+U+V+W", "x<-K+L+N+O+T", "x<-K+L+N+O+U", "x<-K+L+N+O+V", "x<-K+L+N+O+W", "x<-K+L+N+P+T", "x<-K+L+N+P+U", "x<-K+L+N+P+V", "x<-K+L+N+P+W", "x<-K+L+N+R+T", "x<-K+L+N+R+U", "x<-K+L+N+R+V", "x<-K+L+N+R+W", "x<-K+L+N+S+T", "x<-K+L+N+S+U", "x<-K+L+N+S+V", "x<-K+L+N+S+W", "x<-K+L+N+T+U", "x<-K+L+N+T+V", "x<-K+L+N+T+W", "x<-K+L+N+U+V", "x<-K+L+N+U+W", "x<-K+L+N+V+W", "x<-K+L+O+P+T", "x<-K+L+O+P+U", "x<-K+L+O+P+V", "x<-K+L+O+P+W", "x<-K+L+O+Q+T", "x<-K+L+O+Q+U", "x<-K+L+O+Q+V", "x<-K+L+O+Q+W", "x<-K+L+O+S+T", "x<-K+L+O+S+U", "x<-K+L+O+S+V", "x<-K+L+O+S+W", "x<-K+L+O+T+U", "x<-K+L+O+T+V", "x<-K+L+O+T+W", "x<-K+L+O+U+V", "x<-K+L+O+U+W", "x<-K+L+O+V+W", "x<-K+L+P+Q+T", "x<-K+L+P+Q+U", "x<-K+L+P+Q+V", "x<-K+L+P+Q+W", "x<-K+L+P+R+T", "x<-K+L+P+R+U", "x<-K+L+P+R+V", "x<-K+L+P+R+W", "x<-K+L+P+T+U", "x<-K+L+P+T+V", "x<-K+L+P+T+W", "x<-K+L+P+U+V", "x<-K+L+P+U+W", "x<-K+L+P+V+W", "x<-K+L+Q+R+T", "x<-K+L+Q+R+U", "x<-K+L+Q+R+V", "x<-K+L+Q+R+W", "x<-K+L+Q+S+T", "x<-K+L+Q+S+U", "x<-K+L+Q+S+V", "x<-K+L+Q+S+W", "x<-K+L+Q+T+U", "x<-K+L+Q+T+V", "x<-K+L+Q+T+W", "x<-K+L+Q+U+V", "x<-K+L+Q+U+W", "x<-K+L+Q+V+W", "x<-K+L+R+S+T", "x<-K+L+R+S+U", "x<-K+L+R+S+V", "x<-K+L+R+S+W", "x<-K+L+R+T+U", "x<-K+L+R+T+V", "x<-K+L+R+T+W", "x<-K+L+R+U+V", "x<-K+L+R+U+W", "x<-K+L+R+V+W", "x<-K+L+S+T+U", "x<-K+L+S+T+V", "x<-K+L+S+T+W", "x<-K+L+S+U+V", "x<-K+L+S+U+W", "x<-K+L+S+V+W", "x<-K+L+T+U+W", "x<-K+L+T+V+W", "x<-K+L+U+V+W", "x<-K+M+N+O+T", "x<-K+M+N+O+U", "x<-K+M+N+O+V", "x<-K+M+N+O+W", "x<-K+M+N+P+T", "x<-K+M+N+P+U", "x<-K+M+N+P+V", "x<-K+M+N+P+W", "x<-K+M+N+R+T", "x<-K+M+N+R+U", "x<-K+M+N+R+V", "x<-K+M+N+R+W", "x<-K+M+N+S+T", "x<-K+M+N+S+U", "x<-K+M+N+S+V", "x<-K+M+N+S+W", "x<-K+M+N+T+U", "x<-K+M+N+T+V", "x<-K+M+N+T+W", "x<-K+M+N+U+V", "x<-K+M+N+U+W", "x<-K+M+N+V+W", "x<-K+M+O+P+T", "x<-K+M+O+P+U", "x<-K+M+O+P+V", "x<-K+M+O+P+W", "x<-K+M+O+Q+T", "x<-K+M+O+Q+U", "x<-K+M+O+Q+V", "x<-K+M+O+Q+W", "x<-K+M+O+S+T", "x<-K+M+O+S+U", "x<-K+M+O+S+V", "x<-K+M+O+S+W", "x<-K+M+O+T+U", "x<-K+M+O+T+V", "x<-K+M+O+T+W", "x<-K+M+O+U+V", "x<-K+M+O+U+W", "x<-K+M+O+V+W", "x<-K+M+P+Q+T", "x<-K+M+P+Q+U", "x<-K+M+P+Q+V", "x<-K+M+P+Q+W", "x<-K+M+P+R+T", "x<-K+M+P+R+U", "x<-K+M+P+R+V", "x<-K+M+P+R+W", "x<-K+M+P+T+U", "x<-K+M+P+T+V", "x<-K+M+P+T+W", "x<-K+M+P+U+V", "x<-K+M+P+U+W", "x<-K+M+P+V+W", "x<-K+M+Q+R+T", "x<-K+M+Q+R+U", "x<-K+M+Q+R+V", "x<-K+M+Q+R+W", "x<-K+M+Q+S+T", "x<-K+M+Q+S+U", "x<-K+M+Q+S+V", "x<-K+M+Q+S+W", "x<-K+M+Q+T+U", "x<-K+M+Q+T+V", "x<-K+M+Q+T+W", "x<-K+M+Q+U+V", "x<-K+M+Q+U+W", "x<-K+M+Q+V+W", "x<-K+M+R+S+T", "x<-K+M+R+S+U", "x<-K+M+R+S+V", "x<-K+M+R+S+W", "x<-K+M+R+T+U", "x<-K+M+R+T+V", "x<-K+M+R+T+W", "x<-K+M+R+U+V", "x<-K+M+R+U+W", "x<-K+M+R+V+W", "x<-K+M+S+T+U", "x<-K+M+S+T+V", "x<-K+M+S+T+W", "x<-K+M+S+U+V", "x<-K+M+S+U+W", "x<-K+M+S+V+W", "x<-K+M+T+U+W", "x<-K+M+T+V+W", "x<-K+M+U+V+W", "x<-K+N+O+R+T", "x<-K+N+O+R+U", "x<-K+N+O+R+V", "x<-K+N+O+R+W", "x<-K+N+O+S+T", "x<-K+N+O+S+U", "x<-K+N+O+S+V", "x<-K+N+O+S+W", "x<-K+N+O+T+U", "x<-K+N+O+T+V", "x<-K+N+O+T+W", "x<-K+N+O+U+V", "x<-K+N+O+U+W", "x<-K+N+O+V+W", "x<-K+N+P+R+T", "x<-K+N+P+R+U", "x<-K+N+P+R+V", "x<-K+N+P+R+W", "x<-K+N+P+S+T", "x<-K+N+P+S+U", "x<-K+N+P+S+V", "x<-K+N+P+S+W", "x<-K+N+P+T+U", "x<-K+N+P+T+V", "x<-K+N+P+T+W", "x<-K+N+P+U+V", "x<-K+N+P+U+W", "x<-K+N+P+V+W", "x<-K+N+R+T+U", "x<-K+N+R+T+V", "x<-K+N+R+T+W", "x<-K+N+R+U+V", "x<-K+N+R+U+W", "x<-K+N+R+V+W", "x<-K+N+S+T+U", "x<-K+N+S+T+V", "x<-K+N+S+T+W", "x<-K+N+S+U+V", "x<-K+N+S+U+W", "x<-K+N+S+V+W", "x<-K+N+T+U+W", "x<-K+N+T+V+W", "x<-K+N+U+V+W", "x<-K+O+P+S+T", "x<-K+O+P+S+U", "x<-K+O+P+S+V", "x<-K+O+P+S+W", "x<-K+O+P+T+U", "x<-K+O+P+T+V", "x<-K+O+P+T+W", "x<-K+O+P+U+V", "x<-K+O+P+U+W", "x<-K+O+P+V+W", "x<-K+O+Q+R+T", "x<-K+O+Q+R+U", "x<-K+O+Q+R+V", "x<-K+O+Q+R+W", "x<-K+O+Q+S+T", "x<-K+O+Q+S+U", "x<-K+O+Q+S+V", "x<-K+O+Q+S+W", "x<-K+O+Q+T+U", "x<-K+O+Q+T+V", "x<-K+O+Q+T+W", "x<-K+O+Q+U+V", "x<-K+O+Q+U+W", "x<-K+O+Q+V+W", "x<-K+O+R+S+T", "x<-K+O+R+S+U", "x<-K+O+R+S+V", "x<-K+O+R+S+W", "x<-K+O+R+T+U", "x<-K+O+R+T+V", "x<-K+O+R+T+W", "x<-K+O+R+U+V", "x<-K+O+R+U+W", "x<-K+O+R+V+W", "x<-K+O+S+T+U", "x<-K+O+S+T+V", "x<-K+O+S+T+W", "x<-K+O+S+U+V", "x<-K+O+S+U+W", "x<-K+O+S+V+W", "x<-K+O+T+U+W", "x<-K+O+T+V+W", "x<-K+O+U+V+W", "x<-K+P+Q+R+T", "x<-K+P+Q+R+U", "x<-K+P+Q+R+V", "x<-K+P+Q+R+W", "x<-K+P+Q+S+T", "x<-K+P+Q+S+U", "x<-K+P+Q+S+V", "x<-K+P+Q+S+W", "x<-K+P+Q+T+U", "x<-K+P+Q+T+V", "x<-K+P+Q+T+W", "x<-K+P+Q+U+V", "x<-K+P+Q+U+W", "x<-K+P+Q+V+W", "x<-K+P+R+S+T", "x<-K+P+R+S+U", "x<-K+P+R+S+V", "x<-K+P+R+S+W", "x<-K+P+R+T+U", "x<-K+P+R+T+V", "x<-K+P+R+T+W", "x<-K+P+R+U+V", "x<-K+P+R+U+W", "x<-K+P+R+V+W", "x<-K+P+S+T+U", "x<-K+P+S+T+V", "x<-K+P+S+T+W", "x<-K+P+S+U+V", "x<-K+P+S+U+W", "x<-K+P+S+V+W", "x<-K+P+T+U+W", "x<-K+P+T+V+W", "x<-K+P+U+V+W", "x<-K+Q+R+T+U", "x<-K+Q+R+T+V", "x<-K+Q+R+T+W", "x<-K+Q+R+U+V", "x<-K+Q+R+U+W", "x<-K+Q+R+V+W", "x<-K+Q+S+T+U", "x<-K+Q+S+T+V", "x<-K+Q+S+T+W", "x<-K+Q+S+U+V", "x<-K+Q+S+U+W", "x<-K+Q+S+V+W", "x<-K+Q+T+U+W", "x<-K+Q+T+V+W", "x<-K+Q+U+V+W", "x<-K+R+S+T+U", "x<-K+R+S+T+V", "x<-K+R+S+T+W", "x<-K+R+S+U+V", "x<-K+R+S+U+W", "x<-K+R+S+V+W", "x<-K+R+T+U+W", "x<-K+R+T+V+W", "x<-K+R+U+V+W", "x<-K+S+T+U+W", "x<-K+S+T+V+W", "x<-K+S+U+V+W", "x<-L+M+N+O+T", "x<-L+M+N+O+U", "x<-L+M+N+O+V", "x<-L+M+N+O+W", "x<-L+M+N+P+T", "x<-L+M+N+P+U", "x<-L+M+N+P+V", "x<-L+M+N+P+W", "x<-L+M+N+R+T", "x<-L+M+N+R+U", "x<-L+M+N+R+V", "x<-L+M+N+R+W", "x<-L+M+N+S+T", "x<-L+M+N+S+U", "x<-L+M+N+S+V", "x<-L+M+N+S+W", "x<-L+M+N+T+U", "x<-L+M+N+T+V", "x<-L+M+N+T+W", "x<-L+M+N+U+V", "x<-L+M+N+U+W", "x<-L+M+N+V+W", "x<-L+M+O+P+T", "x<-L+M+O+P+U", "x<-L+M+O+P+V", "x<-L+M+O+P+W", "x<-L+M+O+Q+T", "x<-L+M+O+Q+U", "x<-L+M+O+Q+V", "x<-L+M+O+Q+W", "x<-L+M+O+S+T", "x<-L+M+O+S+U", "x<-L+M+O+S+V", "x<-L+M+O+S+W", "x<-L+M+O+T+U", "x<-L+M+O+T+V", "x<-L+M+O+T+W", "x<-L+M+O+U+V", "x<-L+M+O+U+W", "x<-L+M+O+V+W", "x<-L+M+P+Q+T", "x<-L+M+P+Q+U", "x<-L+M+P+Q+V", "x<-L+M+P+Q+W", "x<-L+M+P+R+T", "x<-L+M+P+R+U", "x<-L+M+P+R+V", "x<-L+M+P+R+W", "x<-L+M+P+T+U", "x<-L+M+P+T+V", "x<-L+M+P+T+W", "x<-L+M+P+U+V", "x<-L+M+P+U+W", "x<-L+M+P+V+W", "x<-L+M+Q+R+T", "x<-L+M+Q+R+U", "x<-L+M+Q+R+V", "x<-L+M+Q+R+W", "x<-L+M+Q+S+T", "x<-L+M+Q+S+U", "x<-L+M+Q+S+V", "x<-L+M+Q+S+W", "x<-L+M+Q+T+U", "x<-L+M+Q+T+V", "x<-L+M+Q+T+W", "x<-L+M+Q+U+V", "x<-L+M+Q+U+W", "x<-L+M+Q+V+W", "x<-L+M+R+S+T", "x<-L+M+R+S+U", "x<-L+M+R+S+V", "x<-L+M+R+S+W", "x<-L+M+R+T+U", "x<-L+M+R+T+V", "x<-L+M+R+T+W", "x<-L+M+R+U+V", "x<-L+M+R+U+W", "x<-L+M+R+V+W", "x<-L+M+S+T+U", "x<-L+M+S+T+V", "x<-L+M+S+T+W", "x<-L+M+S+U+V", "x<-L+M+S+U+W", "x<-L+M+S+V+W", "x<-L+M+T+U+W", "x<-L+M+T+V+W", "x<-L+M+U+V+W", "x<-L+N+O+Q+T", "x<-L+N+O+Q+U", "x<-L+N+O+Q+V", "x<-L+N+O+Q+W", "x<-L+N+O+T+U", "x<-L+N+O+T+V", "x<-L+N+O+T+W", "x<-L+N+O+U+V", "x<-L+N+O+U+W", "x<-L+N+O+V+W", "x<-L+N+P+Q+T", "x<-L+N+P+Q+U", "x<-L+N+P+Q+V", "x<-L+N+P+Q+W", "x<-L+N+P+R+T", "x<-L+N+P+R+U", "x<-L+N+P+R+V", "x<-L+N+P+R+W", "x<-L+N+P+S+T", "x<-L+N+P+S+U", "x<-L+N+P+S+V", "x<-L+N+P+S+W", "x<-L+N+P+T+U", "x<-L+N+P+T+V", "x<-L+N+P+T+W", "x<-L+N+P+U+V", "x<-L+N+P+U+W", "x<-L+N+P+V+W", "x<-L+N+Q+R+T", "x<-L+N+Q+R+U", "x<-L+N+Q+R+V", "x<-L+N+Q+R+W", "x<-L+N+Q+S+T", "x<-L+N+Q+S+U", "x<-L+N+Q+S+V", "x<-L+N+Q+S+W", "x<-L+N+Q+T+U", "x<-L+N+Q+T+V", "x<-L+N+Q+T+W", "x<-L+N+Q+U+V", "x<-L+N+Q+U+W", "x<-L+N+Q+V+W", "x<-L+N+R+S+T", "x<-L+N+R+S+U", "x<-L+N+R+S+V", "x<-L+N+R+S+W", "x<-L+N+R+T+U", "x<-L+N+R+T+V", "x<-L+N+R+T+W", "x<-L+N+R+U+V", "x<-L+N+R+U+W", "x<-L+N+R+V+W", "x<-L+N+S+T+U", "x<-L+N+S+T+V", "x<-L+N+S+T+W", "x<-L+N+S+U+V", "x<-L+N+S+U+W", "x<-L+N+S+V+W", "x<-L+N+T+U+W", "x<-L+N+T+V+W", "x<-L+N+U+V+W", "x<-L+O+P+S+T", "x<-L+O+P+S+U", "x<-L+O+P+S+V", "x<-L+O+P+S+W", "x<-L+O+P+T+U", "x<-L+O+P+T+V", "x<-L+O+P+T+W", "x<-L+O+P+U+V", "x<-L+O+P+U+W", "x<-L+O+P+V+W", "x<-L+O+Q+T+U", "x<-L+O+Q+T+V", "x<-L+O+Q+T+W", "x<-L+O+Q+U+V", "x<-L+O+Q+U+W", "x<-L+O+Q+V+W", "x<-L+O+S+T+U", "x<-L+O+S+T+V", "x<-L+O+S+T+W", "x<-L+O+S+U+V", "x<-L+O+S+U+W", "x<-L+O+S+V+W", "x<-L+O+T+U+W", "x<-L+O+T+V+W", "x<-L+O+U+V+W", "x<-L+P+Q+R+T", "x<-L+P+Q+R+U", "x<-L+P+Q+R+V", "x<-L+P+Q+R+W", "x<-L+P+Q+S+T", "x<-L+P+Q+S+U", "x<-L+P+Q+S+V", "x<-L+P+Q+S+W", "x<-L+P+Q+T+U", "x<-L+P+Q+T+V", "x<-L+P+Q+T+W", "x<-L+P+Q+U+V", "x<-L+P+Q+U+W", "x<-L+P+Q+V+W", "x<-L+P+R+S+T", "x<-L+P+R+S+U", "x<-L+P+R+S+V", "x<-L+P+R+S+W", "x<-L+P+R+T+U", "x<-L+P+R+T+V", "x<-L+P+R+T+W", "x<-L+P+R+U+V", "x<-L+P+R+U+W", "x<-L+P+R+V+W", "x<-L+P+S+T+U", "x<-L+P+S+T+V", "x<-L+P+S+T+W", "x<-L+P+S+U+V", "x<-L+P+S+U+W", "x<-L+P+S+V+W", "x<-L+P+T+U+W", "x<-L+P+T+V+W", "x<-L+P+U+V+W", "x<-L+Q+R+T+U", "x<-L+Q+R+T+V", "x<-L+Q+R+T+W", "x<-L+Q+R+U+V", "x<-L+Q+R+U+W", "x<-L+Q+R+V+W", "x<-L+Q+S+T+U", "x<-L+Q+S+T+V", "x<-L+Q+S+T+W", "x<-L+Q+S+U+V", "x<-L+Q+S+U+W", "x<-L+Q+S+V+W", "x<-L+Q+T+U+W", "x<-L+Q+T+V+W", "x<-L+Q+U+V+W", "x<-L+R+S+T+U", "x<-L+R+S+T+V", "x<-L+R+S+T+W", "x<-L+R+S+U+V", "x<-L+R+S+U+W", "x<-L+R+S+V+W", "x<-L+R+T+U+W", "x<-L+R+T+V+W", "x<-L+R+U+V+W", "x<-L+S+T+U+W", "x<-L+S+T+V+W", "x<-L+S+U+V+W", "x<-M+N+O+Q+T", "x<-M+N+O+Q+U", "x<-M+N+O+Q+V", "x<-M+N+O+Q+W", "x<-M+N+O+T+U", "x<-M+N+O+T+V", "x<-M+N+O+T+W", "x<-M+N+O+U+V", "x<-M+N+O+U+W", "x<-M+N+O+V+W", "x<-M+N+P+Q+T", "x<-M+N+P+Q+U", "x<-M+N+P+Q+V", "x<-M+N+P+Q+W", "x<-M+N+P+R+T", "x<-M+N+P+R+U", "x<-M+N+P+R+V", "x<-M+N+P+R+W", "x<-M+N+P+T+U", "x<-M+N+P+T+V", "x<-M+N+P+T+W", "x<-M+N+P+U+V", "x<-M+N+P+U+W", "x<-M+N+P+V+W", "x<-M+N+Q+R+T", "x<-M+N+Q+R+U", "x<-M+N+Q+R+V", "x<-M+N+Q+R+W", "x<-M+N+Q+S+T", "x<-M+N+Q+S+U", "x<-M+N+Q+S+V", "x<-M+N+Q+S+W", "x<-M+N+Q+T+U", "x<-M+N+Q+T+V", "x<-M+N+Q+T+W", "x<-M+N+Q+U+V", "x<-M+N+Q+U+W", "x<-M+N+Q+V+W", "x<-M+N+R+S+T", "x<-M+N+R+S+U", "x<-M+N+R+S+V", "x<-M+N+R+S+W", "x<-M+N+R+T+U", "x<-M+N+R+T+V", "x<-M+N+R+T+W", "x<-M+N+R+U+V", "x<-M+N+R+U+W", "x<-M+N+R+V+W", "x<-M+N+S+T+U", "x<-M+N+S+T+V", "x<-M+N+S+T+W", "x<-M+N+S+U+V", "x<-M+N+S+U+W", "x<-M+N+S+V+W", "x<-M+N+T+U+W", "x<-M+N+T+V+W", "x<-M+N+U+V+W", "x<-M+O+P+Q+T", "x<-M+O+P+Q+U", "x<-M+O+P+Q+V", "x<-M+O+P+Q+W", "x<-M+O+P+R+T", "x<-M+O+P+R+U", "x<-M+O+P+R+V", "x<-M+O+P+R+W", "x<-M+O+P+T+U", "x<-M+O+P+T+V", "x<-M+O+P+T+W", "x<-M+O+P+U+V", "x<-M+O+P+U+W", "x<-M+O+P+V+W", "x<-M+O+Q+R+T", "x<-M+O+Q+R+U", "x<-M+O+Q+R+V", "x<-M+O+Q+R+W", "x<-M+O+Q+S+T", "x<-M+O+Q+S+U", "x<-M+O+Q+S+V", "x<-M+O+Q+S+W", "x<-M+O+Q+T+U", "x<-M+O+Q+T+V", "x<-M+O+Q+T+W", "x<-M+O+Q+U+V", "x<-M+O+Q+U+W", "x<-M+O+Q+V+W", "x<-M+O+R+S+T", "x<-M+O+R+S+U", "x<-M+O+R+S+V", "x<-M+O+R+S+W", "x<-M+O+R+T+U", "x<-M+O+R+T+V", "x<-M+O+R+T+W", "x<-M+O+R+U+V", "x<-M+O+R+U+W", "x<-M+O+R+V+W", "x<-M+O+S+T+U", "x<-M+O+S+T+V", "x<-M+O+S+T+W", "x<-M+O+S+U+V", "x<-M+O+S+U+W", "x<-M+O+S+V+W", "x<-M+O+T+U+W", "x<-M+O+T+V+W", "x<-M+O+U+V+W", "x<-M+P+Q+T+U", "x<-M+P+Q+T+V", "x<-M+P+Q+T+W", "x<-M+P+Q+U+V", "x<-M+P+Q+U+W", "x<-M+P+Q+V+W", "x<-M+P+R+T+U", "x<-M+P+R+T+V", "x<-M+P+R+T+W", "x<-M+P+R+U+V", "x<-M+P+R+U+W", "x<-M+P+R+V+W", "x<-M+P+T+U+W", "x<-M+P+T+V+W", "x<-M+P+U+V+W", "x<-M+Q+R+T+U", "x<-M+Q+R+T+V", "x<-M+Q+R+T+W", "x<-M+Q+R+U+V", "x<-M+Q+R+U+W", "x<-M+Q+R+V+W", "x<-M+Q+S+T+U", "x<-M+Q+S+T+V", "x<-M+Q+S+T+W", "x<-M+Q+S+U+V", "x<-M+Q+S+U+W", "x<-M+Q+S+V+W", "x<-M+Q+T+U+W", "x<-M+Q+T+V+W", "x<-M+Q+U+V+W", "x<-M+R+S+T+U", "x<-M+R+S+T+V", "x<-M+R+S+T+W", "x<-M+R+S+U+V", "x<-M+R+S+U+W", "x<-M+R+S+V+W", "x<-M+R+T+U+W", "x<-M+R+T+V+W", "x<-M+R+U+V+W", "x<-M+S+T+U+W", "x<-M+S+T+V+W", "x<-M+S+U+V+W", "x<-N+O+Q+R+T", "x<-N+O+Q+R+U", "x<-N+O+Q+R+V", "x<-N+O+Q+R+W", "x<-N+O+Q+S+T", "x<-N+O+Q+S+U", "x<-N+O+Q+S+V", "x<-N+O+Q+S+W", "x<-N+O+Q+T+U", "x<-N+O+Q+T+V", "x<-N+O+Q+T+W", "x<-N+O+Q+U+V", "x<-N+O+Q+U+W", "x<-N+O+Q+V+W", "x<-N+O+R+S+T", "x<-N+O+R+S+U", "x<-N+O+R+S+V", "x<-N+O+R+S+W", "x<-N+O+R+T+U", "x<-N+O+R+T+V", "x<-N+O+R+T+W", "x<-N+O+R+U+V", "x<-N+O+R+U+W", "x<-N+O+R+V+W", "x<-N+O+S+T+U", "x<-N+O+S+T+V", "x<-N+O+S+T+W", "x<-N+O+S+U+V", "x<-N+O+S+U+W", "x<-N+O+S+V+W", "x<-N+O+T+U+W", "x<-N+O+T+V+W", "x<-N+O+U+V+W", "x<-N+P+Q+R+T", "x<-N+P+Q+R+U", "x<-N+P+Q+R+V", "x<-N+P+Q+R+W", "x<-N+P+Q+S+T", "x<-N+P+Q+S+U", "x<-N+P+Q+S+V", "x<-N+P+Q+S+W", "x<-N+P+Q+T+U", "x<-N+P+Q+T+V", "x<-N+P+Q+T+W", "x<-N+P+Q+U+V", "x<-N+P+Q+U+W", "x<-N+P+Q+V+W", "x<-N+P+R+S+T", "x<-N+P+R+S+U", "x<-N+P+R+S+V", "x<-N+P+R+S+W", "x<-N+P+R+T+U", "x<-N+P+R+T+V", "x<-N+P+R+T+W", "x<-N+P+R+U+V", "x<-N+P+R+U+W", "x<-N+P+R+V+W", "x<-N+P+S+T+U", "x<-N+P+S+T+V", "x<-N+P+S+T+W", "x<-N+P+S+U+V", "x<-N+P+S+U+W", "x<-N+P+S+V+W", "x<-N+P+T+U+W", "x<-N+P+T+V+W", "x<-N+P+U+V+W", "x<-N+Q+R+T+U", "x<-N+Q+R+T+V", "x<-N+Q+R+T+W", "x<-N+Q+R+U+V", "x<-N+Q+R+U+W", "x<-N+Q+R+V+W", "x<-N+Q+S+T+U", "x<-N+Q+S+T+V", "x<-N+Q+S+T+W", "x<-N+Q+S+U+V", "x<-N+Q+S+U+W", "x<-N+Q+S+V+W", "x<-N+Q+T+U+W", "x<-N+Q+T+V+W", "x<-N+Q+U+V+W", "x<-N+R+S+T+U", "x<-N+R+S+T+V", "x<-N+R+S+T+W", "x<-N+R+S+U+V", "x<-N+R+S+U+W", "x<-N+R+S+V+W", "x<-N+R+T+U+W", "x<-N+R+T+V+W", "x<-N+R+U+V+W", "x<-N+S+T+U+W", "x<-N+S+T+V+W", "x<-N+S+U+V+W", "x<-O+P+Q+R+T", "x<-O+P+Q+R+U", "x<-O+P+Q+R+V", "x<-O+P+Q+R+W", "x<-O+P+Q+S+T", "x<-O+P+Q+S+U", "x<-O+P+Q+S+V", "x<-O+P+Q+S+W", "x<-O+P+Q+T+U", "x<-O+P+Q+T+V", "x<-O+P+Q+T+W", "x<-O+P+Q+U+V", "x<-O+P+Q+U+W", "x<-O+P+Q+V+W", "x<-O+P+R+S+T", "x<-O+P+R+S+U", "x<-O+P+R+S+V", "x<-O+P+R+S+W", "x<-O+P+R+T+U", "x<-O+P+R+T+V", "x<-O+P+R+T+W", "x<-O+P+R+U+V", "x<-O+P+R+U+W", "x<-O+P+R+V+W", "x<-O+P+S+T+U", "x<-O+P+S+T+V", "x<-O+P+S+T+W", "x<-O+P+S+U+V", "x<-O+P+S+U+W", "x<-O+P+S+V+W", "x<-O+P+T+U+W", "x<-O+P+T+V+W", "x<-O+P+U+V+W", "x<-O+Q+R+T+U", "x<-O+Q+R+T+V", "x<-O+Q+R+T+W", "x<-O+Q+R+U+V", "x<-O+Q+R+U+W", "x<-O+Q+R+V+W", "x<-O+Q+S+T+U", "x<-O+Q+S+T+V", "x<-O+Q+S+T+W", "x<-O+Q+S+U+V", "x<-O+Q+S+U+W", "x<-O+Q+S+V+W", "x<-O+Q+T+U+W", "x<-O+Q+T+V+W", "x<-O+Q+U+V+W", "x<-O+R+S+T+U", "x<-O+R+S+T+V", "x<-O+R+S+T+W", "x<-O+R+S+U+V", "x<-O+R+S+U+W", "x<-O+R+S+V+W", "x<-O+R+T+U+W", "x<-O+R+T+V+W", "x<-O+R+U+V+W", "x<-O+S+T+U+W", "x<-O+S+T+V+W", "x<-O+S+U+V+W", "x<-P+Q+R+T+U", "x<-P+Q+R+T+V", "x<-P+Q+R+T+W", "x<-P+Q+R+U+V", "x<-P+Q+R+U+W", "x<-P+Q+R+V+W", "x<-P+Q+S+T+U", "x<-P+Q+S+T+V", "x<-P+Q+S+T+W", "x<-P+Q+S+U+V", "x<-P+Q+S+U+W", "x<-P+Q+S+V+W", "x<-P+Q+T+U+W", "x<-P+Q+T+V+W", "x<-P+Q+U+V+W", "x<-P+R+S+T+U", "x<-P+R+S+T+V", "x<-P+R+S+T+W", "x<-P+R+S+U+V", "x<-P+R+S+U+W", "x<-P+R+S+V+W", "x<-P+R+T+U+W", "x<-P+R+T+V+W", "x<-P+R+U+V+W", "x<-P+S+T+U+W", "x<-P+S+T+V+W", "x<-P+S+U+V+W", "x<-Q+R+T+U+W", "x<-Q+R+T+V+W", "x<-Q+R+U+V+W", "x<-Q+S+T+U+W", "x<-Q+S+T+V+W", "x<-Q+S+U+V+W", "x<-R+S+T+U+W", "x<-R+S+T+V+W", "x<-R+S+U+V+W", "x<-K+L+N+O+T+U", "x<-K+L+N+O+T+V", "x<-K+L+N+O+T+W", "x<-K+L+N+O+U+V", "x<-K+L+N+O+U+W", "x<-K+L+N+O+V+W", "x<-K+L+N+P+T+U", "x<-K+L+N+P+T+V", "x<-K+L+N+P+T+W", "x<-K+L+N+P+U+V", "x<-K+L+N+P+U+W", "x<-K+L+N+P+V+W", "x<-K+L+N+R+T+U", "x<-K+L+N+R+T+V", "x<-K+L+N+R+T+W", "x<-K+L+N+R+U+V", "x<-K+L+N+R+U+W", "x<-K+L+N+R+V+W", "x<-K+L+N+S+T+U", "x<-K+L+N+S+T+V", "x<-K+L+N+S+T+W", "x<-K+L+N+S+U+V", "x<-K+L+N+S+U+W", "x<-K+L+N+S+V+W", "x<-K+L+N+T+U+W", "x<-K+L+N+T+V+W", "x<-K+L+N+U+V+W", "x<-K+L+O+P+T+U", "x<-K+L+O+P+T+V", "x<-K+L+O+P+T+W", "x<-K+L+O+P+U+V", "x<-K+L+O+P+U+W", "x<-K+L+O+P+V+W", "x<-K+L+O+Q+T+U", "x<-K+L+O+Q+T+V", "x<-K+L+O+Q+T+W", "x<-K+L+O+Q+U+V", "x<-K+L+O+Q+U+W", "x<-K+L+O+Q+V+W", "x<-K+L+O+S+T+U", "x<-K+L+O+S+T+V", "x<-K+L+O+S+T+W", "x<-K+L+O+S+U+V", "x<-K+L+O+S+U+W", "x<-K+L+O+S+V+W", "x<-K+L+O+T+U+W", "x<-K+L+O+T+V+W", "x<-K+L+O+U+V+W", "x<-K+L+P+Q+T+U", "x<-K+L+P+Q+T+V", "x<-K+L+P+Q+T+W", "x<-K+L+P+Q+U+V", "x<-K+L+P+Q+U+W", "x<-K+L+P+Q+V+W", "x<-K+L+P+R+T+U", "x<-K+L+P+R+T+V", "x<-K+L+P+R+T+W", "x<-K+L+P+R+U+V", "x<-K+L+P+R+U+W", "x<-K+L+P+R+V+W", "x<-K+L+P+T+U+W", "x<-K+L+P+T+V+W", "x<-K+L+P+U+V+W", "x<-K+L+Q+R+T+U", "x<-K+L+Q+R+T+V", "x<-K+L+Q+R+T+W", "x<-K+L+Q+R+U+V", "x<-K+L+Q+R+U+W", "x<-K+L+Q+R+V+W", "x<-K+L+Q+S+T+U", "x<-K+L+Q+S+T+V", "x<-K+L+Q+S+T+W", "x<-K+L+Q+S+U+V", "x<-K+L+Q+S+U+W", "x<-K+L+Q+S+V+W", "x<-K+L+Q+T+U+W", "x<-K+L+Q+T+V+W", "x<-K+L+Q+U+V+W", "x<-K+L+R+S+T+U", "x<-K+L+R+S+T+V", "x<-K+L+R+S+T+W", "x<-K+L+R+S+U+V", "x<-K+L+R+S+U+W", "x<-K+L+R+S+V+W", "x<-K+L+R+T+U+W", "x<-K+L+R+T+V+W", "x<-K+L+R+U+V+W", "x<-K+L+S+T+U+W", "x<-K+L+S+T+V+W", "x<-K+L+S+U+V+W", "x<-K+M+N+O+T+U", "x<-K+M+N+O+T+V", "x<-K+M+N+O+T+W", "x<-K+M+N+O+U+V", "x<-K+M+N+O+U+W", "x<-K+M+N+O+V+W", "x<-K+M+N+P+T+U", "x<-K+M+N+P+T+V", "x<-K+M+N+P+T+W", "x<-K+M+N+P+U+V", "x<-K+M+N+P+U+W", "x<-K+M+N+P+V+W", "x<-K+M+N+R+T+U", "x<-K+M+N+R+T+V", "x<-K+M+N+R+T+W", "x<-K+M+N+R+U+V", "x<-K+M+N+R+U+W", "x<-K+M+N+R+V+W", "x<-K+M+N+S+T+U", "x<-K+M+N+S+T+V", "x<-K+M+N+S+T+W", "x<-K+M+N+S+U+V", "x<-K+M+N+S+U+W", "x<-K+M+N+S+V+W", "x<-K+M+N+T+U+W", "x<-K+M+N+T+V+W", "x<-K+M+N+U+V+W", "x<-K+M+O+P+T+U", "x<-K+M+O+P+T+V", "x<-K+M+O+P+T+W", "x<-K+M+O+P+U+V", "x<-K+M+O+P+U+W", "x<-K+M+O+P+V+W", "x<-K+M+O+Q+T+U", "x<-K+M+O+Q+T+V", "x<-K+M+O+Q+T+W", "x<-K+M+O+Q+U+V", "x<-K+M+O+Q+U+W", "x<-K+M+O+Q+V+W", "x<-K+M+O+S+T+U", "x<-K+M+O+S+T+V", "x<-K+M+O+S+T+W", "x<-K+M+O+S+U+V", "x<-K+M+O+S+U+W", "x<-K+M+O+S+V+W", "x<-K+M+O+T+U+W", "x<-K+M+O+T+V+W", "x<-K+M+O+U+V+W", "x<-K+M+P+Q+T+U", "x<-K+M+P+Q+T+V", "x<-K+M+P+Q+T+W", "x<-K+M+P+Q+U+V", "x<-K+M+P+Q+U+W", "x<-K+M+P+Q+V+W", "x<-K+M+P+R+T+U", "x<-K+M+P+R+T+V", "x<-K+M+P+R+T+W", "x<-K+M+P+R+U+V", "x<-K+M+P+R+U+W", "x<-K+M+P+R+V+W", "x<-K+M+P+T+U+W", "x<-K+M+P+T+V+W", "x<-K+M+P+U+V+W", "x<-K+M+Q+R+T+U", "x<-K+M+Q+R+T+V", "x<-K+M+Q+R+T+W", "x<-K+M+Q+R+U+V", "x<-K+M+Q+R+U+W", "x<-K+M+Q+R+V+W", "x<-K+M+Q+S+T+U", "x<-K+M+Q+S+T+V", "x<-K+M+Q+S+T+W", "x<-K+M+Q+S+U+V", "x<-K+M+Q+S+U+W", "x<-K+M+Q+S+V+W", "x<-K+M+Q+T+U+W", "x<-K+M+Q+T+V+W", "x<-K+M+Q+U+V+W", "x<-K+M+R+S+T+U", "x<-K+M+R+S+T+V", "x<-K+M+R+S+T+W", "x<-K+M+R+S+U+V", "x<-K+M+R+S+U+W", "x<-K+M+R+S+V+W", "x<-K+M+R+T+U+W", "x<-K+M+R+T+V+W", "x<-K+M+R+U+V+W", "x<-K+M+S+T+U+W", "x<-K+M+S+T+V+W", "x<-K+M+S+U+V+W", "x<-K+N+O+R+T+U", "x<-K+N+O+R+T+V", "x<-K+N+O+R+T+W", "x<-K+N+O+R+U+V", "x<-K+N+O+R+U+W", "x<-K+N+O+R+V+W", "x<-K+N+O+S+T+U", "x<-K+N+O+S+T+V", "x<-K+N+O+S+T+W", "x<-K+N+O+S+U+V", "x<-K+N+O+S+U+W", "x<-K+N+O+S+V+W", "x<-K+N+O+T+U+W", "x<-K+N+O+T+V+W", "x<-K+N+O+U+V+W", "x<-K+N+P+R+T+U", "x<-K+N+P+R+T+V", "x<-K+N+P+R+T+W", "x<-K+N+P+R+U+V", "x<-K+N+P+R+U+W", "x<-K+N+P+R+V+W", "x<-K+N+P+S+T+U", "x<-K+N+P+S+T+V", "x<-K+N+P+S+T+W", "x<-K+N+P+S+U+V", "x<-K+N+P+S+U+W", "x<-K+N+P+S+V+W", "x<-K+N+P+T+U+W", "x<-K+N+P+T+V+W", "x<-K+N+P+U+V+W", "x<-K+N+R+T+U+W", "x<-K+N+R+T+V+W", "x<-K+N+R+U+V+W", "x<-K+N+S+T+U+W", "x<-K+N+S+T+V+W", "x<-K+N+S+U+V+W", "x<-K+O+P+S+T+U", "x<-K+O+P+S+T+V", "x<-K+O+P+S+T+W", "x<-K+O+P+S+U+V", "x<-K+O+P+S+U+W", "x<-K+O+P+S+V+W", "x<-K+O+P+T+U+W", "x<-K+O+P+T+V+W", "x<-K+O+P+U+V+W", "x<-K+O+Q+R+T+U", "x<-K+O+Q+R+T+V", "x<-K+O+Q+R+T+W", "x<-K+O+Q+R+U+V", "x<-K+O+Q+R+U+W", "x<-K+O+Q+R+V+W", "x<-K+O+Q+S+T+U", "x<-K+O+Q+S+T+V", "x<-K+O+Q+S+T+W", "x<-K+O+Q+S+U+V", "x<-K+O+Q+S+U+W", "x<-K+O+Q+S+V+W", "x<-K+O+Q+T+U+W", "x<-K+O+Q+T+V+W", "x<-K+O+Q+U+V+W", "x<-K+O+R+S+T+U", "x<-K+O+R+S+T+V", "x<-K+O+R+S+T+W", "x<-K+O+R+S+U+V", "x<-K+O+R+S+U+W", "x<-K+O+R+S+V+W", "x<-K+O+R+T+U+W", "x<-K+O+R+T+V+W", "x<-K+O+R+U+V+W", "x<-K+O+S+T+U+W", "x<-K+O+S+T+V+W", "x<-K+O+S+U+V+W", "x<-K+P+Q+R+T+U", "x<-K+P+Q+R+T+V", "x<-K+P+Q+R+T+W", "x<-K+P+Q+R+U+V", "x<-K+P+Q+R+U+W", "x<-K+P+Q+R+V+W", "x<-K+P+Q+S+T+U", "x<-K+P+Q+S+T+V", "x<-K+P+Q+S+T+W", "x<-K+P+Q+S+U+V", "x<-K+P+Q+S+U+W", "x<-K+P+Q+S+V+W", "x<-K+P+Q+T+U+W", "x<-K+P+Q+T+V+W", "x<-K+P+Q+U+V+W", "x<-K+P+R+S+T+U", "x<-K+P+R+S+T+V", "x<-K+P+R+S+T+W", "x<-K+P+R+S+U+V", "x<-K+P+R+S+U+W", "x<-K+P+R+S+V+W", "x<-K+P+R+T+U+W", "x<-K+P+R+T+V+W", "x<-K+P+R+U+V+W", "x<-K+P+S+T+U+W", "x<-K+P+S+T+V+W", "x<-K+P+S+U+V+W", "x<-K+Q+R+T+U+W", "x<-K+Q+R+T+V+W", "x<-K+Q+R+U+V+W", "x<-K+Q+S+T+U+W", "x<-K+Q+S+T+V+W", "x<-K+Q+S+U+V+W", "x<-K+R+S+T+U+W", "x<-K+R+S+T+V+W", "x<-K+R+S+U+V+W", "x<-L+M+N+O+T+U", "x<-L+M+N+O+T+V", "x<-L+M+N+O+T+W", "x<-L+M+N+O+U+V", "x<-L+M+N+O+U+W", "x<-L+M+N+O+V+W", "x<-L+M+N+P+T+U", "x<-L+M+N+P+T+V", "x<-L+M+N+P+T+W", "x<-L+M+N+P+U+V", "x<-L+M+N+P+U+W", "x<-L+M+N+P+V+W", "x<-L+M+N+R+T+U", "x<-L+M+N+R+T+V", "x<-L+M+N+R+T+W", "x<-L+M+N+R+U+V", "x<-L+M+N+R+U+W", "x<-L+M+N+R+V+W", "x<-L+M+N+S+T+U", "x<-L+M+N+S+T+V", "x<-L+M+N+S+T+W", "x<-L+M+N+S+U+V", "x<-L+M+N+S+U+W", "x<-L+M+N+S+V+W", "x<-L+M+N+T+U+W", "x<-L+M+N+T+V+W", "x<-L+M+N+U+V+W", "x<-L+M+O+P+T+U", "x<-L+M+O+P+T+V", "x<-L+M+O+P+T+W", "x<-L+M+O+P+U+V", "x<-L+M+O+P+U+W", "x<-L+M+O+P+V+W", "x<-L+M+O+Q+T+U", "x<-L+M+O+Q+T+V", "x<-L+M+O+Q+T+W", "x<-L+M+O+Q+U+V", "x<-L+M+O+Q+U+W", "x<-L+M+O+Q+V+W", "x<-L+M+O+S+T+U", "x<-L+M+O+S+T+V", "x<-L+M+O+S+T+W", "x<-L+M+O+S+U+V", "x<-L+M+O+S+U+W", "x<-L+M+O+S+V+W", "x<-L+M+O+T+U+W", "x<-L+M+O+T+V+W", "x<-L+M+O+U+V+W", "x<-L+M+P+Q+T+U", "x<-L+M+P+Q+T+V", "x<-L+M+P+Q+T+W", "x<-L+M+P+Q+U+V", "x<-L+M+P+Q+U+W", "x<-L+M+P+Q+V+W", "x<-L+M+P+R+T+U", "x<-L+M+P+R+T+V", "x<-L+M+P+R+T+W", "x<-L+M+P+R+U+V", "x<-L+M+P+R+U+W", "x<-L+M+P+R+V+W", "x<-L+M+P+T+U+W", "x<-L+M+P+T+V+W", "x<-L+M+P+U+V+W", "x<-L+M+Q+R+T+U", "x<-L+M+Q+R+T+V", "x<-L+M+Q+R+T+W", "x<-L+M+Q+R+U+V", "x<-L+M+Q+R+U+W", "x<-L+M+Q+R+V+W", "x<-L+M+Q+S+T+U", "x<-L+M+Q+S+T+V", "x<-L+M+Q+S+T+W", "x<-L+M+Q+S+U+V", "x<-L+M+Q+S+U+W", "x<-L+M+Q+S+V+W", "x<-L+M+Q+T+U+W", "x<-L+M+Q+T+V+W", "x<-L+M+Q+U+V+W", "x<-L+M+R+S+T+U", "x<-L+M+R+S+T+V", "x<-L+M+R+S+T+W", "x<-L+M+R+S+U+V", "x<-L+M+R+S+U+W", "x<-L+M+R+S+V+W", "x<-L+M+R+T+U+W", "x<-L+M+R+T+V+W", "x<-L+M+R+U+V+W", "x<-L+M+S+T+U+W", "x<-L+M+S+T+V+W", "x<-L+M+S+U+V+W", "x<-L+N+O+Q+T+U", "x<-L+N+O+Q+T+V", "x<-L+N+O+Q+T+W", "x<-L+N+O+Q+U+V", "x<-L+N+O+Q+U+W", "x<-L+N+O+Q+V+W", "x<-L+N+O+T+U+W", "x<-L+N+O+T+V+W", "x<-L+N+O+U+V+W", "x<-L+N+P+Q+T+U", "x<-L+N+P+Q+T+V", "x<-L+N+P+Q+T+W", "x<-L+N+P+Q+U+V", "x<-L+N+P+Q+U+W", "x<-L+N+P+Q+V+W", "x<-L+N+P+R+T+U", "x<-L+N+P+R+T+V", "x<-L+N+P+R+T+W", "x<-L+N+P+R+U+V", "x<-L+N+P+R+U+W", "x<-L+N+P+R+V+W", "x<-L+N+P+S+T+U", "x<-L+N+P+S+T+V", "x<-L+N+P+S+T+W", "x<-L+N+P+S+U+V", "x<-L+N+P+S+U+W", "x<-L+N+P+S+V+W", "x<-L+N+P+T+U+W", "x<-L+N+P+T+V+W", "x<-L+N+P+U+V+W", "x<-L+N+Q+R+T+U", "x<-L+N+Q+R+T+V", "x<-L+N+Q+R+T+W", "x<-L+N+Q+R+U+V", "x<-L+N+Q+R+U+W", "x<-L+N+Q+R+V+W", "x<-L+N+Q+S+T+U", "x<-L+N+Q+S+T+V", "x<-L+N+Q+S+T+W", "x<-L+N+Q+S+U+V", "x<-L+N+Q+S+U+W", "x<-L+N+Q+S+V+W", "x<-L+N+Q+T+U+W", "x<-L+N+Q+T+V+W", "x<-L+N+Q+U+V+W", "x<-L+N+R+S+T+U", "x<-L+N+R+S+T+V", "x<-L+N+R+S+T+W", "x<-L+N+R+S+U+V", "x<-L+N+R+S+U+W", "x<-L+N+R+S+V+W", "x<-L+N+R+T+U+W", "x<-L+N+R+T+V+W", "x<-L+N+R+U+V+W", "x<-L+N+S+T+U+W", "x<-L+N+S+T+V+W", "x<-L+N+S+U+V+W", "x<-L+O+P+S+T+U", "x<-L+O+P+S+T+V", "x<-L+O+P+S+T+W", "x<-L+O+P+S+U+V", "x<-L+O+P+S+U+W", "x<-L+O+P+S+V+W", "x<-L+O+P+T+U+W", "x<-L+O+P+T+V+W", "x<-L+O+P+U+V+W", "x<-L+O+Q+T+U+W", "x<-L+O+Q+T+V+W", "x<-L+O+Q+U+V+W", "x<-L+O+S+T+U+W", "x<-L+O+S+T+V+W", "x<-L+O+S+U+V+W", "x<-L+P+Q+R+T+U", "x<-L+P+Q+R+T+V", "x<-L+P+Q+R+T+W", "x<-L+P+Q+R+U+V", "x<-L+P+Q+R+U+W", "x<-L+P+Q+R+V+W", "x<-L+P+Q+S+T+U", "x<-L+P+Q+S+T+V", "x<-L+P+Q+S+T+W", "x<-L+P+Q+S+U+V", "x<-L+P+Q+S+U+W", "x<-L+P+Q+S+V+W", "x<-L+P+Q+T+U+W", "x<-L+P+Q+T+V+W", "x<-L+P+Q+U+V+W", "x<-L+P+R+S+T+U", "x<-L+P+R+S+T+V", "x<-L+P+R+S+T+W", "x<-L+P+R+S+U+V", "x<-L+P+R+S+U+W", "x<-L+P+R+S+V+W", "x<-L+P+R+T+U+W", "x<-L+P+R+T+V+W", "x<-L+P+R+U+V+W", "x<-L+P+S+T+U+W", "x<-L+P+S+T+V+W", "x<-L+P+S+U+V+W", "x<-L+Q+R+T+U+W", "x<-L+Q+R+T+V+W", "x<-L+Q+R+U+V+W", "x<-L+Q+S+T+U+W", "x<-L+Q+S+T+V+W", "x<-L+Q+S+U+V+W", "x<-L+R+S+T+U+W", "x<-L+R+S+T+V+W", "x<-L+R+S+U+V+W", "x<-M+N+O+Q+T+U", "x<-M+N+O+Q+T+V", "x<-M+N+O+Q+T+W", "x<-M+N+O+Q+U+V", "x<-M+N+O+Q+U+W", "x<-M+N+O+Q+V+W", "x<-M+N+O+T+U+W", "x<-M+N+O+T+V+W", "x<-M+N+O+U+V+W", "x<-M+N+P+Q+T+U", "x<-M+N+P+Q+T+V", "x<-M+N+P+Q+T+W", "x<-M+N+P+Q+U+V", "x<-M+N+P+Q+U+W", "x<-M+N+P+Q+V+W", "x<-M+N+P+R+T+U", "x<-M+N+P+R+T+V", "x<-M+N+P+R+T+W", "x<-M+N+P+R+U+V", "x<-M+N+P+R+U+W", "x<-M+N+P+R+V+W", "x<-M+N+P+T+U+W", "x<-M+N+P+T+V+W", "x<-M+N+P+U+V+W", "x<-M+N+Q+R+T+U", "x<-M+N+Q+R+T+V", "x<-M+N+Q+R+T+W", "x<-M+N+Q+R+U+V", "x<-M+N+Q+R+U+W", "x<-M+N+Q+R+V+W", "x<-M+N+Q+S+T+U", "x<-M+N+Q+S+T+V", "x<-M+N+Q+S+T+W", "x<-M+N+Q+S+U+V", "x<-M+N+Q+S+U+W", "x<-M+N+Q+S+V+W", "x<-M+N+Q+T+U+W", "x<-M+N+Q+T+V+W", "x<-M+N+Q+U+V+W", "x<-M+N+R+S+T+U", "x<-M+N+R+S+T+V", "x<-M+N+R+S+T+W", "x<-M+N+R+S+U+V", "x<-M+N+R+S+U+W", "x<-M+N+R+S+V+W", "x<-M+N+R+T+U+W", "x<-M+N+R+T+V+W", "x<-M+N+R+U+V+W", "x<-M+N+S+T+U+W", "x<-M+N+S+T+V+W", "x<-M+N+S+U+V+W", "x<-M+O+P+Q+T+U", "x<-M+O+P+Q+T+V", "x<-M+O+P+Q+T+W", "x<-M+O+P+Q+U+V", "x<-M+O+P+Q+U+W", "x<-M+O+P+Q+V+W", "x<-M+O+P+R+T+U", "x<-M+O+P+R+T+V", "x<-M+O+P+R+T+W", "x<-M+O+P+R+U+V", "x<-M+O+P+R+U+W", "x<-M+O+P+R+V+W", "x<-M+O+P+T+U+W", "x<-M+O+P+T+V+W", "x<-M+O+P+U+V+W", "x<-M+O+Q+R+T+U", "x<-M+O+Q+R+T+V", "x<-M+O+Q+R+T+W", "x<-M+O+Q+R+U+V", "x<-M+O+Q+R+U+W", "x<-M+O+Q+R+V+W", "x<-M+O+Q+S+T+U", "x<-M+O+Q+S+T+V", "x<-M+O+Q+S+T+W", "x<-M+O+Q+S+U+V", "x<-M+O+Q+S+U+W", "x<-M+O+Q+S+V+W", "x<-M+O+Q+T+U+W", "x<-M+O+Q+T+V+W", "x<-M+O+Q+U+V+W", "x<-M+O+R+S+T+U", "x<-M+O+R+S+T+V", "x<-M+O+R+S+T+W", "x<-M+O+R+S+U+V", "x<-M+O+R+S+U+W", "x<-M+O+R+S+V+W", "x<-M+O+R+T+U+W", "x<-M+O+R+T+V+W", "x<-M+O+R+U+V+W", "x<-M+O+S+T+U+W", "x<-M+O+S+T+V+W", "x<-M+O+S+U+V+W", "x<-M+P+Q+T+U+W", "x<-M+P+Q+T+V+W", "x<-M+P+Q+U+V+W", "x<-M+P+R+T+U+W", "x<-M+P+R+T+V+W", "x<-M+P+R+U+V+W", "x<-M+Q+R+T+U+W", "x<-M+Q+R+T+V+W", "x<-M+Q+R+U+V+W", "x<-M+Q+S+T+U+W", "x<-M+Q+S+T+V+W", "x<-M+Q+S+U+V+W", "x<-M+R+S+T+U+W", "x<-M+R+S+T+V+W", "x<-M+R+S+U+V+W", "x<-N+O+Q+R+T+U", "x<-N+O+Q+R+T+V", "x<-N+O+Q+R+T+W", "x<-N+O+Q+R+U+V", "x<-N+O+Q+R+U+W", "x<-N+O+Q+R+V+W", "x<-N+O+Q+S+T+U", "x<-N+O+Q+S+T+V", "x<-N+O+Q+S+T+W", "x<-N+O+Q+S+U+V", "x<-N+O+Q+S+U+W", "x<-N+O+Q+S+V+W", "x<-N+O+Q+T+U+W", "x<-N+O+Q+T+V+W", "x<-N+O+Q+U+V+W", "x<-N+O+R+S+T+U", "x<-N+O+R+S+T+V", "x<-N+O+R+S+T+W", "x<-N+O+R+S+U+V", "x<-N+O+R+S+U+W", "x<-N+O+R+S+V+W", "x<-N+O+R+T+U+W", "x<-N+O+R+T+V+W", "x<-N+O+R+U+V+W", "x<-N+O+S+T+U+W", "x<-N+O+S+T+V+W", "x<-N+O+S+U+V+W", "x<-N+P+Q+R+T+U", "x<-N+P+Q+R+T+V", "x<-N+P+Q+R+T+W", "x<-N+P+Q+R+U+V", "x<-N+P+Q+R+U+W", "x<-N+P+Q+R+V+W", "x<-N+P+Q+S+T+U", "x<-N+P+Q+S+T+V", "x<-N+P+Q+S+T+W", "x<-N+P+Q+S+U+V", "x<-N+P+Q+S+U+W", "x<-N+P+Q+S+V+W", "x<-N+P+Q+T+U+W", "x<-N+P+Q+T+V+W", "x<-N+P+Q+U+V+W", "x<-N+P+R+S+T+U", "x<-N+P+R+S+T+V", "x<-N+P+R+S+T+W", "x<-N+P+R+S+U+V", "x<-N+P+R+S+U+W", "x<-N+P+R+S+V+W", "x<-N+P+R+T+U+W", "x<-N+P+R+T+V+W", "x<-N+P+R+U+V+W", "x<-N+P+S+T+U+W", "x<-N+P+S+T+V+W", "x<-N+P+S+U+V+W", "x<-N+Q+R+T+U+W", "x<-N+Q+R+T+V+W", "x<-N+Q+R+U+V+W", "x<-N+Q+S+T+U+W", "x<-N+Q+S+T+V+W", "x<-N+Q+S+U+V+W", "x<-N+R+S+T+U+W", "x<-N+R+S+T+V+W", "x<-N+R+S+U+V+W", "x<-O+P+Q+R+T+U", "x<-O+P+Q+R+T+V", "x<-O+P+Q+R+T+W", "x<-O+P+Q+R+U+V", "x<-O+P+Q+R+U+W", "x<-O+P+Q+R+V+W", "x<-O+P+Q+S+T+U", "x<-O+P+Q+S+T+V", "x<-O+P+Q+S+T+W", "x<-O+P+Q+S+U+V", "x<-O+P+Q+S+U+W", "x<-O+P+Q+S+V+W", "x<-O+P+Q+T+U+W", "x<-O+P+Q+T+V+W", "x<-O+P+Q+U+V+W", "x<-O+P+R+S+T+U", "x<-O+P+R+S+T+V", "x<-O+P+R+S+T+W", "x<-O+P+R+S+U+V", "x<-O+P+R+S+U+W", "x<-O+P+R+S+V+W", "x<-O+P+R+T+U+W", "x<-O+P+R+T+V+W", "x<-O+P+R+U+V+W", "x<-O+P+S+T+U+W", "x<-O+P+S+T+V+W", "x<-O+P+S+U+V+W", "x<-O+Q+R+T+U+W", "x<-O+Q+R+T+V+W", "x<-O+Q+R+U+V+W", "x<-O+Q+S+T+U+W", "x<-O+Q+S+T+V+W", "x<-O+Q+S+U+V+W", "x<-O+R+S+T+U+W", "x<-O+R+S+T+V+W", "x<-O+R+S+U+V+W", "x<-P+Q+R+T+U+W", "x<-P+Q+R+T+V+W", "x<-P+Q+R+U+V+W", "x<-P+Q+S+T+U+W", "x<-P+Q+S+T+V+W", "x<-P+Q+S+U+V+W", "x<-P+R+S+T+U+W", "x<-P+R+S+T+V+W", "x<-P+R+S+U+V+W", "x<-K+L+N+O+T+U+W", "x<-K+L+N+O+T+V+W", "x<-K+L+N+O+U+V+W", "x<-K+L+N+P+T+U+W", "x<-K+L+N+P+T+V+W", "x<-K+L+N+P+U+V+W", "x<-K+L+N+R+T+U+W", "x<-K+L+N+R+T+V+W", "x<-K+L+N+R+U+V+W", "x<-K+L+N+S+T+U+W", "x<-K+L+N+S+T+V+W", "x<-K+L+N+S+U+V+W", "x<-K+L+O+P+T+U+W", "x<-K+L+O+P+T+V+W", "x<-K+L+O+P+U+V+W", "x<-K+L+O+Q+T+U+W", "x<-K+L+O+Q+T+V+W", "x<-K+L+O+Q+U+V+W", "x<-K+L+O+S+T+U+W", "x<-K+L+O+S+T+V+W", "x<-K+L+O+S+U+V+W", "x<-K+L+P+Q+T+U+W", "x<-K+L+P+Q+T+V+W", "x<-K+L+P+Q+U+V+W", "x<-K+L+P+R+T+U+W", "x<-K+L+P+R+T+V+W", "x<-K+L+P+R+U+V+W", "x<-K+L+Q+R+T+U+W", "x<-K+L+Q+R+T+V+W", "x<-K+L+Q+R+U+V+W", "x<-K+L+Q+S+T+U+W", "x<-K+L+Q+S+T+V+W", "x<-K+L+Q+S+U+V+W", "x<-K+L+R+S+T+U+W", "x<-K+L+R+S+T+V+W", "x<-K+L+R+S+U+V+W", "x<-K+M+N+O+T+U+W", "x<-K+M+N+O+T+V+W", "x<-K+M+N+O+U+V+W", "x<-K+M+N+P+T+U+W", "x<-K+M+N+P+T+V+W", "x<-K+M+N+P+U+V+W", "x<-K+M+N+R+T+U+W", "x<-K+M+N+R+T+V+W", "x<-K+M+N+R+U+V+W", "x<-K+M+N+S+T+U+W", "x<-K+M+N+S+T+V+W", "x<-K+M+N+S+U+V+W", "x<-K+M+O+P+T+U+W", "x<-K+M+O+P+T+V+W", "x<-K+M+O+P+U+V+W", "x<-K+M+O+Q+T+U+W", "x<-K+M+O+Q+T+V+W", "x<-K+M+O+Q+U+V+W", "x<-K+M+O+S+T+U+W", "x<-K+M+O+S+T+V+W", "x<-K+M+O+S+U+V+W", "x<-K+M+P+Q+T+U+W", "x<-K+M+P+Q+T+V+W", "x<-K+M+P+Q+U+V+W", "x<-K+M+P+R+T+U+W", "x<-K+M+P+R+T+V+W", "x<-K+M+P+R+U+V+W", "x<-K+M+Q+R+T+U+W", "x<-K+M+Q+R+T+V+W", "x<-K+M+Q+R+U+V+W", "x<-K+M+Q+S+T+U+W", "x<-K+M+Q+S+T+V+W", "x<-K+M+Q+S+U+V+W", "x<-K+M+R+S+T+U+W", "x<-K+M+R+S+T+V+W", "x<-K+M+R+S+U+V+W", "x<-K+N+O+R+T+U+W", "x<-K+N+O+R+T+V+W", "x<-K+N+O+R+U+V+W", "x<-K+N+O+S+T+U+W", "x<-K+N+O+S+T+V+W", "x<-K+N+O+S+U+V+W", "x<-K+N+P+R+T+U+W", "x<-K+N+P+R+T+V+W", "x<-K+N+P+R+U+V+W", "x<-K+N+P+S+T+U+W", "x<-K+N+P+S+T+V+W", "x<-K+N+P+S+U+V+W", "x<-K+O+P+S+T+U+W", "x<-K+O+P+S+T+V+W", "x<-K+O+P+S+U+V+W", "x<-K+O+Q+R+T+U+W", "x<-K+O+Q+R+T+V+W", "x<-K+O+Q+R+U+V+W", "x<-K+O+Q+S+T+U+W", "x<-K+O+Q+S+T+V+W", "x<-K+O+Q+S+U+V+W", "x<-K+O+R+S+T+U+W", "x<-K+O+R+S+T+V+W", "x<-K+O+R+S+U+V+W", "x<-K+P+Q+R+T+U+W", "x<-K+P+Q+R+T+V+W", "x<-K+P+Q+R+U+V+W", "x<-K+P+Q+S+T+U+W", "x<-K+P+Q+S+T+V+W", "x<-K+P+Q+S+U+V+W", "x<-K+P+R+S+T+U+W", "x<-K+P+R+S+T+V+W", "x<-K+P+R+S+U+V+W", "x<-L+M+N+O+T+U+W", "x<-L+M+N+O+T+V+W", "x<-L+M+N+O+U+V+W", "x<-L+M+N+P+T+U+W", "x<-L+M+N+P+T+V+W", "x<-L+M+N+P+U+V+W", "x<-L+M+N+R+T+U+W", "x<-L+M+N+R+T+V+W", "x<-L+M+N+R+U+V+W", "x<-L+M+N+S+T+U+W", "x<-L+M+N+S+T+V+W", "x<-L+M+N+S+U+V+W", "x<-L+M+O+P+T+U+W", "x<-L+M+O+P+T+V+W", "x<-L+M+O+P+U+V+W", "x<-L+M+O+Q+T+U+W", "x<-L+M+O+Q+T+V+W", "x<-L+M+O+Q+U+V+W", "x<-L+M+O+S+T+U+W", "x<-L+M+O+S+T+V+W", "x<-L+M+O+S+U+V+W", "x<-L+M+P+Q+T+U+W", "x<-L+M+P+Q+T+V+W", "x<-L+M+P+Q+U+V+W", "x<-L+M+P+R+T+U+W", "x<-L+M+P+R+T+V+W", "x<-L+M+P+R+U+V+W", "x<-L+M+Q+R+T+U+W", "x<-L+M+Q+R+T+V+W", "x<-L+M+Q+R+U+V+W", "x<-L+M+Q+S+T+U+W", "x<-L+M+Q+S+T+V+W", "x<-L+M+Q+S+U+V+W", "x<-L+M+R+S+T+U+W", "x<-L+M+R+S+T+V+W", "x<-L+M+R+S+U+V+W", "x<-L+N+O+Q+T+U+W", "x<-L+N+O+Q+T+V+W", "x<-L+N+O+Q+U+V+W", "x<-L+N+P+Q+T+U+W", "x<-L+N+P+Q+T+V+W", "x<-L+N+P+Q+U+V+W", "x<-L+N+P+R+T+U+W", "x<-L+N+P+R+T+V+W", "x<-L+N+P+R+U+V+W", "x<-L+N+P+S+T+U+W", "x<-L+N+P+S+T+V+W", "x<-L+N+P+S+U+V+W", "x<-L+N+Q+R+T+U+W", "x<-L+N+Q+R+T+V+W", "x<-L+N+Q+R+U+V+W", "x<-L+N+Q+S+T+U+W", "x<-L+N+Q+S+T+V+W", "x<-L+N+Q+S+U+V+W", "x<-L+N+R+S+T+U+W", "x<-L+N+R+S+T+V+W", "x<-L+N+R+S+U+V+W", "x<-L+O+P+S+T+U+W", "x<-L+O+P+S+T+V+W", "x<-L+O+P+S+U+V+W", "x<-L+P+Q+R+T+U+W", "x<-L+P+Q+R+T+V+W", "x<-L+P+Q+R+U+V+W", "x<-L+P+Q+S+T+U+W", "x<-L+P+Q+S+T+V+W", "x<-L+P+Q+S+U+V+W", "x<-L+P+R+S+T+U+W", "x<-L+P+R+S+T+V+W", "x<-L+P+R+S+U+V+W", "x<-M+N+O+Q+T+U+W", "x<-M+N+O+Q+T+V+W", "x<-M+N+O+Q+U+V+W", "x<-M+N+P+Q+T+U+W", "x<-M+N+P+Q+T+V+W", "x<-M+N+P+Q+U+V+W", "x<-M+N+P+R+T+U+W", "x<-M+N+P+R+T+V+W", "x<-M+N+P+R+U+V+W", "x<-M+N+Q+R+T+U+W", "x<-M+N+Q+R+T+V+W", "x<-M+N+Q+R+U+V+W", "x<-M+N+Q+S+T+U+W", "x<-M+N+Q+S+T+V+W", "x<-M+N+Q+S+U+V+W", "x<-M+N+R+S+T+U+W", "x<-M+N+R+S+T+V+W", "x<-M+N+R+S+U+V+W", "x<-M+O+P+Q+T+U+W", "x<-M+O+P+Q+T+V+W", "x<-M+O+P+Q+U+V+W", "x<-M+O+P+R+T+U+W", "x<-M+O+P+R+T+V+W", "x<-M+O+P+R+U+V+W", "x<-M+O+Q+R+T+U+W", "x<-M+O+Q+R+T+V+W", "x<-M+O+Q+R+U+V+W", "x<-M+O+Q+S+T+U+W", "x<-M+O+Q+S+T+V+W", "x<-M+O+Q+S+U+V+W", "x<-M+O+R+S+T+U+W", "x<-M+O+R+S+T+V+W", "x<-M+O+R+S+U+V+W", "x<-N+O+Q+R+T+U+W", "x<-N+O+Q+R+T+V+W", "x<-N+O+Q+R+U+V+W", "x<-N+O+Q+S+T+U+W", "x<-N+O+Q+S+T+V+W", "x<-N+O+Q+S+U+V+W", "x<-N+O+R+S+T+U+W", "x<-N+O+R+S+T+V+W", "x<-N+O+R+S+U+V+W", "x<-N+P+Q+R+T+U+W", "x<-N+P+Q+R+T+V+W", "x<-N+P+Q+R+U+V+W", "x<-N+P+Q+S+T+U+W", "x<-N+P+Q+S+T+V+W", "x<-N+P+Q+S+U+V+W", "x<-N+P+R+S+T+U+W", "x<-N+P+R+S+T+V+W", "x<-N+P+R+S+U+V+W", "x<-O+P+Q+R+T+U+W", "x<-O+P+Q+R+T+V+W", "x<-O+P+Q+R+U+V+W", "x<-O+P+Q+S+T+U+W", "x<-O+P+Q+S+T+V+W", "x<-O+P+Q+S+U+V+W", "x<-O+P+R+S+T+U+W", "x<-O+P+R+S+T+V+W", "x<-O+P+R+S+U+V+W",
			});

		// Does not work with 36 conditions, memory explodes -> instead dynamically get during runtime
		//   labeling.generate_rules(GetActionFromRuleIndex);

		return labeling;
	}

	action_bitset GetActionFromRuleIndex(const rule_set& rs, uint64_t rule_index) const override {
		rule_wrapper r(rs, rule_index);

		bool X = r[Xa] || r[Xb] || r[Xc] || r[Xd] || r[Xe] || r[Xf] || r[Xg] || r[Xh];
		if (!X) {
			//r << "nothing";
			return action_bitset(1).set(0);
		}

		const bool K = r[Kh];
		const bool L = r[Lg] || r[Lh];
		const bool M = r[Mg];
		const bool N = r[Nf] || r[Nh];
		const bool O = r[Oe] || r[Of] || r[Og] || r[Oh];
		const bool P = r[Pe] || r[Pg];
		const bool Q = r[Qf];
		const bool R = r[Re] || r[Rf];
		const bool S = r[Se];
		const bool T = r[Td] || r[Th];
		const bool U = r[Uc] || r[Ud] || r[Ug] || r[Uh];
		const bool V = r[Vc] || r[Vg];
		const bool W = r[Wb] || r[Wd] || r[Wf] || r[Wh];

		//int lookup_value = (K << 0) | (L << 1) | (M << 2) | (N << 3) | (O << 4) | (P << 5) | (Q << 6) | (R << 7) | (S << 8) | (T << 9) | (U << 10) | (V << 11) | (W << 12);
		//return function_pointer_table[lookup_value](r, rs);
		return GetActions(r, rs, K, L, M, N, O, P, Q, R, S, T, U, V, W);
	}

};
