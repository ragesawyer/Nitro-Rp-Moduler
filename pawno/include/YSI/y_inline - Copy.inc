/*----------------------------------------------------------------------------*\
					===================================
					 y_inline - PAWN inline functions.
					===================================
Description:
	This library allows a user to write inline functions in their script.  It
	first detects all inline functions and generates data on them, such as
	parameter counts and addresses.  When an inline function is passed in code
	its current context data is stored.  Finally, when an inline function is
	found to be called at some point its current local stack is stored in global
	memory.  When the function actually is called, the stack is restored, and
	additional parameters which are the inline function parameters, are passed.
Legal:
	Version: MPL 1.1
	
	The contents of this file are subject to the Mozilla Public License Version 
	1.1 (the "License"); you may not use this file except in compliance with 
	the License. You may obtain a copy of the License at 
	http://www.mozilla.org/MPL/
	
	Software distributed under the License is distributed on an "AS IS" basis,
	WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
	for the specific language governing rights and limitations under the
	License.
	
	The Original Code is the YSI AMX include.
	
	The Initial Developer of the Original Code is Alex "Y_Less" Cole.
	Portions created by the Initial Developer are Copyright (C) 2011
	the Initial Developer. All Rights Reserved.
	
	Contributors:
		ZeeX, koolk, JoeBullet/Google63, g_aSlice/Slice
	
	Thanks:
		JoeBullet/Google63 - Handy arbitrary ASM jump code using SCTRL.
		ZeeX - Very productive conversations.
		koolk - IsPlayerinAreaEx code.
		TheAlpha - Danish translation.
		breadfish - German translation.
		Fireburn - Dutch translation.
		yom - French translation.
		50p - Polish translation.
		Zamaroht - Spanish translation.
		Dracoblue, sintax, mabako, Xtreme, other coders - Producing other modes
			for me to strive to better.
		Pixels^ - Running XScripters where the idea was born.
		Matite - Pestering me to release it and using it.
	
	Very special thanks to:
		Thiadmer - PAWN, whose limits continue to amaze me!
		Kye/Kalcor - SA:MP.
		SA:MP Team past, present and future - SA:MP.
	
Version:
	1.0
Changelog:
	19/09/11:
		First version
\*----------------------------------------------------------------------------*/

#include <YSI\internal\y_version>
#include <YSI\internal\y_funcinc>
#include <YSI\y_amx>
#include <YSI\y_utils>
#include <YSI\y_malloc>
#include <YSI\y_hooks>

#if defined YSI_MALLOC_SECURE
	#error y_inline does not work with "YSI_MALLOC_SECURE" defined.
#endif

// "with inline X"
// "with public X"
// %0 = " in" or " pub" (ignored).
// "%1 = "ne X" or "c X" (makes a macro).

#define using%0) callback_tag:@Ik:@Il:%0)

// Get ONLY this ONE parameter (this is a VERY important stage)!
#define @Ik:@Il:%0, @Ip:@Iq:@Im:@Io:|||%0|||,
#define @Il:%0) @Ip:@Iq:@Im:@Io:|||%0|||)

// You can use "using InlineFunc" or the faster "using inline InlineFunc".
#define @Ip:@Iq:@Im:@Io:|||%0anonymous%1||| @Iu:@Iv:%0NULL%1|||
#define @Iq:@Im:@Io:|||%0callback%1||| @Ir:@Is:%1|||
#define @Im:@Io:|||%0inline%1||| @Iu:@Iv:%0%1|||
#define @Io:|||%0public%1||| @Ir:@Is:%1|||

// Callbacks with "On" in the name (often overidden by y_hooks and ALS).
#define @Ir:@Is:%0On%1||| @In:@It:#%0"On"#%1|||
#define @Is:%0||| @In:@It:#%0||| //Using_unknown_callback

// Callbacks with additional parameters (MUST have matching parameters (y_ini)).
#define @In:@It:%0(%1)|||%2) %0%2,.bExtra=true,.extra=%1)
#define @It:%0||| %0

// Inline function surpressing closures.
#define @Iu:@Iv:%0$%1||| (J@=0,_:@In:@It:%1||| _Y_INLINE_END
#define @Iv:%1||| (J@=1,_:@In:@It:%1||| _Y_INLINE_END

// Defer adding the close bracket till after other macros have run.
#define _Y_INLINE_END )

#define INLINE_LOOP_PATTERN_0 0xA1E7C013
#define INLINE_LOOP_PATTERN_1 0x42424242
#define INLINE_LOOP_PATTERN_2 0x13C0E7A1
#define INLINE_LOOP_PATTERN_3 0x21495359

#define INLINE_LOOP_PATTERNS INLINE_LOOP_PATTERN_0,INLINE_LOOP_PATTERN_1,INLINE_LOOP_PATTERN_2,INLINE_LOOP_PATTERN_3

// This code uses a specialisation of my "tag macros" technique, embedding the
// macro names in to a string instead of using them as tags.  This does mean
// that they will likely end up in the final AMX (unlike tag macros) but this
// seems to be the only way to get this to work that I can see as there is no
// variable reading involved.

// The "inline" macro only has ":...", whereas the "anonymous" macro has ":...:"
// because "inline" gets its second colon later on after all the parameter tag-
// style macros.  The structure for these extra cells is:
//  
//  0 - Colon.
//  1 - Pointer to next function name.
//  2 - Pointer the the function.
//  3 - Cells in the enclosing function at this point (passed and declared).
//  4 - Parameters format.
//  
#define inline%0(%1) static const%0[]=#%0":..."#_YI@CP;for(new%1,,;Inline_Loop(INLINE_LOOP_PATTERNS,%0);)

#define callback:%0) const callback_tag:callback_check:%0���
#define callback_tag:callback_check:%0,%1��� callback_tag:%0[],%1)
#define callback_check:%0��� %0[])

#define anonymous%0(%1) Q@=#_YI@CA;for(new%1,,;Inline_Loop(INLINE_LOOP_PATTERNS,":...:"#);)

#define _YI@CP;for(new%0,%1; @Ia#@Ib#@Ic#@Id#:;for(new @Iz|||%0|||%1;

#define _YI@CA;for(new%0,%1; @If#@Ig#@Ih#@Ii#:;for(new @Iz|||%0|||%1;

// Detect 0 parameters.
#define @Ia#@Ib#@Ic#@Id#:;for(new%0||||||,;%1;) :;for(;%1;)

// Detect strings (no array support YET).
#define @Ib#@Ic#@Id#%9;for(new%0|||%1string:%2[%3]|||%4,%5; @Ib#@Ic#@Id#%9s;for(new%0,%2[YSI_MAX_STRING]|||%4|||%5;
// Detect end of line.
#define @Ic#@Id#%9;for(new%0|||%2|||%4,%5; @Ib#@Ic#@Id#%9i;for(new%0,%2|||%4|||%5;
// Detect everything else.
#define @Id#%9;for(new%0||||||; %9;for(new%0;

// Drop the leading comma on the parameter list.
#define @Iz,

// Detect 0 parameters.
#define @If#@Ig#@Ih#@Ii#:;for(new%0||||||,;%1;) :;for(;%1;)

// Detect strings (no array support YET).
#define @Ig#@Ih#@Ii#%9;for(new%0|||%1string:%2[%3]|||%4,%5;Inline_Loop(%6#%7) @Ig#@Ih#@Ii#%9;for(new%0,%2[YSI_MAX_STRING]|||%4|||%5;Inline_Loop(%6#%7s)
// Detect end of line.
#define @Ih#@Ii#%9;for(new%0|||%2|||%4,%5;Inline_Loop(%6#%7) @Ig#@Ih#@Ii#%9;for(new%0,%2|||%4|||%5;Inline_Loop(%6#%7i)
// Detect everything else.
#define @Ii#%9;for(new%0||||||; ;for(new%0;

// Drop the leading comma on the parameter list.
#define @Iz,

static stock
	YSI_g_sFirstFunc = -1,
	YSI_g_sInInline = 0,
	//YSI_g_sLastFunc = -1,
	YSI_g_sReturn;

enum e_CALLBACK_FLAGS (<<= 1)
{
	// All this is required to correctly design the call stack.
	e_CALLBACK_FLAGS_PUSHED  = 0x000000FF, // Parameters pushed to caller.
	e_CALLBACK_FLAGS_CREATED = 0x0FFFFF00, // Data size declared in caller.
	e_CALLBACK_FLAGS_PUBLIC  = 0x10000000  // Is a public function.
	//e_CALLBACK_FLAGS_INLINE                // Is an inline function.
}

enum E_CALLBACK_DATA
{
	e_CALLBACK_FLAGS:E_CALLBACK_DATA_FLAGS,
	E_CALLBACK_DATA_POINTER, // Pointer to the function.
	E_CALLBACK_DATA_FORMAT,  // Compressed destination format.
	Alloc:E_CALLBACK_DATA_ALLOC // Where our closure is stored.
}

static stock Inline_DoFormat(data[])
{
	// This function encodes the data format in to a single cell.  The format is:
	//  
	//  1111111001
	//  
	// Leading 1s indicate no data.  The 0 immediately following the leading 1s
	// indicates the start of the format section (but is not PART of the format
	// section).  The remaining bits represent either strings (1) or non-strings
	// (0).  For example "(a, string:b, c)" would be:
	//  
	//  1..10010
	//  
	// Where "1..1" indicates full-cell padding of 1s.  From this it is known that
	// the function takes three parameters: non-string, string, non-string.  In
	// addition, as strings in inline functions MUST use the "string:" tag, it is
	// known that ALL strings will ALWAYS be 128 (or "YSI_MAX_STRING") cells big.
	new
		pos = strfind(data, ":"),
		len = strlen(data),
		bit = 1,
		total = -1;
	P:C(if (len - pos - 1 > 30) P:E("Inline functions only support up to 30 parameters"););
	if (pos != -1)
	{
		while (--len != pos)
		{
			if (data[len] != 's')
			{
				total &= ~bit;
			}
			bit <<= 1;
		}
		total &= ~bit;
	}
	// Store the compressed format, also instantly end the string.
	data[0] = total;
	data[1] = '\0';
	return 1;
}

static stock Inline_FindFunction(const data[], const name[], address)
{
	new
		value,
		len = strlen(name),
		tmp,
		candidate = cellmax,
		ret = -1;
	#emit LOAD.S.pri data
	#emit STOR.S.pri value
	// Check the "pointer" is valid.
	//printf("value = %08x, %s, %s", value, data, name);
	while (value != -1)
	{
		if (strcmp(name, data, false, len) && data[len] == ':')
		{
			value = data[strlen(data) - 4];
		}
		else
		{
			// Found a candidate.  Now only finds the closest match BEFORE the call.
			tmp = address - data[len + 2];
			//if (0 < tmp < candidate)
			if (-5000 < tmp < 5000)
			{
				// Constrain the checks to +-5000 because of square integer limits and
				// to help reduce clashes.
				if ((tmp *= tmp) < candidate)
				{
					ret = value;
					candidate = tmp;
				}
			}
			value = data[len + 1];
		}
		// Move on to the next "pointer".
		#emit LOAD.S.pri value
		#emit STOR.S.pri data
	}
	return ret;
}

static stock Inline_FindAnonymous(const data[], address)
{
	new
		value,
		tmp,
		candidate = cellmax,
		ret = -1;
	#emit LOAD.S.pri data
	#emit STOR.S.pri value
	// Check the "pointer" is valid.
	while (value != -1)
	{
		// Check if this is anonymous.
		if (data[0] == ':')
		{
			// Found a candidate.
			tmp = data[2] - address;
			// Make sure this is the closest anonymous function AFTER the return.  We
			// don't need "0 <=" as it will never be INSTANTLY after the return
			// address due to the required "Inline_Loop" call.
			if (0 < tmp < candidate)
			{
				ret = value;
				candidate = tmp;
			}
		}
		value = data[strlen(data) - 3];
		// Move on to the next "pointer".
		#emit LOAD.S.pri value
		#emit STOR.S.pri data
	}
	return ret;
}

forward _Inline_FixCompiler@@();

public _Inline_FixCompiler@@()
{
	// Call the function above at least once so the address exists in tables.  But
	// never ACTUALLY call it at run-time (don't call this public function).
	Inline_DoFormat("");
	Inline_FindFunction("", "", 0);
	Inline_FindAnonymous("", 0);
	memcpy("", "", 0, 0, 0);
}

hook OnScriptInit()
{
	static
		sSearch[] =
			{
				AMX_PUSH_C, INLINE_LOOP_PATTERN_3,
				AMX_PUSH_C, INLINE_LOOP_PATTERN_2,
				AMX_PUSH_C, INLINE_LOOP_PATTERN_1,
				AMX_PUSH_C, INLINE_LOOP_PATTERN_0,
				AMX_PUSH_C, 0x14,
				AMX_CALL
			};
	new
		addr,
		data,
		func,
		last = -1;
	while (AMX_TraceCode(sSearch, addr, func))
	{
		// Get the function return address (make sure "pri" is always non-zero).
		func = addr + (12 * 4);
		//printf("stored function at %x", func);
		// Get the address of the last parameter.
		addr += (AMX_HEADER_COD - 4);
		// Get the value of the last parameter.
		#emit LREF.S.pri        addr
		#emit STOR.S.pri        data
		// PERFECT!  This assembly code worked FIRST TIME to correctly display
		// both the address and contents of the loaded string!  Amazingly it
		// worked for anonymous functions too...
		//#emit PUSH.S            data
		//printf("data = %08x, %s", data);
		//#emit POP.pri
		// Anyway, now we need to see if this is a named or anonymous function
		// and plan accordingly.  We don't need all the complex code of the
		// previous inline version.  If we get a function call which takes an
		// anonymous function, just assume it is the next one found in the list
		// of stored inline function addresses.  This has the HUGE added
		// advantage of allowing small bits of extra code to appear between the
		// function call and the inline function - i.e. we can allow return
		// values and allsorts now (including having functions taking anonymous
		// functions themselves being used as parameters).
		//new
		//	pos = strfind(
		static const
			scSearch[] = ":";
		new
			pos = 0;
		#emit PUSH.C         0
		#emit PUSH.C         0
		#emit PUSH.C         scSearch
		#emit PUSH.S         data
		#emit PUSH.C         16
		#emit SYSREQ.C       strfind
		#emit STOR.S.pri     pos
		#emit STACK          20
		if (pos != -1)
		{
			if (last == -1)
			{
				YSI_g_sFirstFunc = data;
			}
			else
			{
				#emit LOAD.S.pri data
				#emit SREF.S.pri last
			}
			// Equivalent to: "data[pos + 1] = -1;" (1 cell = 4 bytes).
			data += pos * 4 + 4;
			#emit CONST.pri    0xFFFFFFFF
			#emit SREF.S.pri   data
			// Equivalent to: "data[pos + 2] = func;"
			last = data;
			data += 4;
			#emit LOAD.S.pri   func
			#emit SREF.S.pri   data
			// Now find and compress the format specifier (backwards).
			// Now compress the format in to a single cell (up to 32 parameters).
			#emit LOAD.S.pri     data
			#emit ADD.C          8//4
			#emit PUSH.pri
			#emit PUSH.C         4
			// Using "CALL Inline_DoFormat" doesn't work, so do the next best thing.
			#emit LCTRL          6
			#emit ADD.C          28
			#emit PUSH.pri
			#emit CONST.pri      Inline_DoFormat
			#emit SCTRL          6
		}
		// Move on to find the next value.
		addr -= (AMX_HEADER_COD - 8);
	}
}

stock Inline_Loop(p0, p1, p2, p3, volatile const format[])
{
	#emit LOAD.S.pri    4
	#emit STOR.S.pri    p0
	if (YSI_g_sInInline == p0)
	{
		#emit LOAD.S.alt 0
		#emit MOVE.pri
		#emit ADD.C      4
		#emit LOAD.I
		#emit XCHG
		#emit LOAD.I
		#emit SCTRL      5
		#emit MOVE.pri
		#emit SCTRL      6
	}
	static const
		scSearch[] = ":";
	// This function needs to be modified to store the stack size at this point
	// and write it to the relevant slot (easy since the relevant slot is
	// passed).  I know "volatile const" makes no sense, but "const" is for the
	// compiler, "volatile" is to show that really it does change.
	#pragma unused p0, p1, p2, p3
	#emit LOAD.S.pri    0
	#emit ADD.C         8
	#emit LOAD.I
	#emit PUSH.pri
	// Get the local variable sizes.  Need to allocate the data somewhere.  First
	#emit LCTRL         5
	#emit LOAD.S.alt    0
	// Subtract the parameters passed to this function.
	#emit ADD.C         32 // 5 * 4 + 12
	#emit SUB.alt
	#emit PUSH.pri
	// Do strfind.
	#emit PUSH.C        0
	#emit PUSH.C        0
	#emit PUSH.C        scSearch
	#emit PUSH.S        format
	#emit PUSH.C        16
	#emit SYSREQ.C      strfind
	#emit STACK         20
	// Save the data.
	#emit CONST.alt     4
	#emit SMUL
	#emit ADD.C         12
	#emit LOAD.S.alt    format
	#emit ADD
	#emit STOR.S.pri    format
	#emit POP.alt
	#emit SHL.C.alt     6
	#emit POP.pri
	#emit SHR.C.pri     2
	#emit ADD
	#emit SREF.S.pri    format
	return 0;
}

stock Callback_Get(callback:name, result[E_CALLBACK_DATA], expect = -1)
{
	new
		func,
		num,
		pos;
	if (isnull(_:name))
	{
		// Anonymous inline.  Need to find the next available inline function based
		// on the return address of the calling function.
		// Get the return address.
		#emit LOAD.S.pri 0
		#emit ADD.C      4
		#emit LOAD.I
		// Call the function.
		#emit PUSH.pri
		#emit PUSH.S     name
		#emit PUSH       YSI_g_sFirstFunc
		#emit PUSH.C     8
		#emit LCTRL      6
		#emit ADD.C      28
		#emit PUSH.pri
		#emit CONST.pri  Inline_FindAnonymous
		#emit SCTRL      6
		#emit STOR.S.pri func
		if (func == -1)
		{
			return 0;
		}
		// Save the data.
		func += 2;
		#emit LREF.S.pri func
		#emit STOR.S.pri pos
		result[E_CALLBACK_DATA_POINTER] = pos;
		// Save the function parameters.
		++func;
		#emit LREF.S.pri func
		#emit STOR.S.pri pos
		result[E_CALLBACK_DATA_FLAGS]  = e_CALLBACK_FLAGS:pos;
		++func;
		#emit LREF.S.pri func
		#emit STOR.S.pri pos
		result[E_CALLBACK_DATA_FORMAT]  = pos;
		if (expect != -1 && pos != expect)
		{
			P:E("Format sepecifier didn't match on anonymous function");
		}
		new
			mask = 0x80000000;
		// Skip leading 1s.
		while (pos & mask)
		{
			mask >>>= 1;
		}
		// Skip delimiting 0.
		mask >>>= 1;
		while (mask)
		{
			if (pos & mask)
			{
				num += YSI_MAX_STRING;
			}
			else
			{
				++num;
			}
			mask >>>= 1;
		}
	}
	else
	{
		pos = strfind(name, ":");
		P:5("Callback_Get: %d, %d, %d, %d, %04x%04x", name[pos + 1], name[pos + 2], name[pos + 3] >>> 8, name[pos + 3] & 0xFF, name[pos + 4] >>> 16, name[pos + 4] & 0xFFFF);
		if (pos == -1)
		{
			if (AMX_GetPublicPointer(0, pos, name))
			{
				// Public function, use standard callback techniques (well, psudo-
				// standard, just store the address and use SCTRL manipulation).
				result[E_CALLBACK_DATA_POINTER] = pos;
				result[E_CALLBACK_DATA_FLAGS]   = e_CALLBACK_FLAGS_PUBLIC;
				result[E_CALLBACK_DATA_FORMAT]  = expect;
				return 1;
			}
			else
			{
				// Get the caller frame.
				#emit LOAD.S.pri 0
				// Get the caller return.
				#emit ADD.C      4
				#emit LOAD.I
				// Now find the closest item with the correct name.  Hopefully 99% of
				// the time there will only be one function with this name anywhere
				// NEAR the return address, so we can use that one.  Otherwise we will
				// just have to hope that the closest is correct (maybe add a check to
				// see if it's too close, and if so alert the user).
				#emit PUSH.pri
				#emit PUSH.S     name
				#emit PUSH       YSI_g_sFirstFunc
				#emit PUSH.C     8
				#emit LCTRL      6
				#emit ADD.C      28
				#emit PUSH.pri
				#emit CONST.pri  Inline_FindFunction
				#emit SCTRL      6
				#emit STOR.S.pri func
				// So now "func" is the address of the handle to the nearest data we can
				// extract all the relevant data.
				if (func == -1)
				{
					return 0;
				}
				// Save the function pointer.
				func += strlen(name) + 2;
				#emit LREF.S.pri func
				#emit STOR.S.pri pos
				result[E_CALLBACK_DATA_POINTER] = pos;
				// Save the function parameters.
				++func;
				#emit LREF.S.pri func
				#emit STOR.S.pri pos
				result[E_CALLBACK_DATA_FLAGS]  = e_CALLBACK_FLAGS:pos;
				// Save the function format.
				++func;
				#emit LREF.S.pri func
				#emit STOR.S.pri pos
				result[E_CALLBACK_DATA_FORMAT]  = pos;
				if (expect != -1 && pos != expect)
				{
					P:E("Format sepecifier didn't match on inline function %s", name);
				}
				new
					mask = 0x80000000;
				// Skip leading 1s.
				while (pos & mask)
				{
					mask >>>= 1;
				}
				// Skip delimiting 0.
				mask >>>= 1;
				while (mask)
				{
					if (pos & mask)
					{
						num += YSI_MAX_STRING;
					}
					else
					{
						++num;
					}
					mask >>>= 1;
				}
			}
		}
		else
		{
			// Named and qualified inline function.  Should also include the correct
			// addresses.  By FAR the fastest method as we already have all the data.
			new
				form = name[pos + 4];
			result[E_CALLBACK_DATA_FLAGS]   = e_CALLBACK_FLAGS:name[pos + 3];
			result[E_CALLBACK_DATA_POINTER] = name[pos + 2];
			result[E_CALLBACK_DATA_FORMAT]  = form;
			if (expect != -1 && pos != form)
			{
				P:E("Format sepecifier didn't match on inline function %s", name);
			}
			// Get the size of inline function parameters:
			new
				mask = 0x80000000;
			// Skip leading 1s.
			while (form & mask)
			{
				mask >>>= 1;
			}
			// Skip delimiting 0.
			mask >>>= 1;
			while (mask)
			{
				if (form & mask)
				{
					num += YSI_MAX_STRING;
				}
				else
				{
					++num;
				}
				mask >>>= 1;
			}
		}
	}
	// Now we need to somehow store all this data somewhere (including, for
	// speed, the extra data involved in calling a function).  Here "pos" is the
	// number of bytes pushed to the owning function.
	result[E_CALLBACK_DATA_FLAGS] -= e_CALLBACK_FLAGS:(num << 8);
	pos = _:result[E_CALLBACK_DATA_FLAGS];
	// Get the size of the closure.
	func = (pos & 0xFF);
	pos = (pos >>> 8); // - num;
	func = func + pos + 3;
	new
		Alloc:alloc = malloc(func);
	if (alloc == NO_ALLOC)
	{
		return 0;
	}
	result[E_CALLBACK_DATA_ALLOC] = alloc;
	// Now we need to copy the data from the previous-but-one frame to this
	// allocated location.  Copy the whole lot, including passed parameters.
	#emit LOAD.S.pri          pos
	#emit SMUL.C              4
	#emit MOVE.alt
	#emit LOAD.S.pri          0
	#emit LOAD.I
	#emit SUB
	#emit STOR.S.pri          name
	memcpy(YSI_gMallocMemory[_:alloc], name, 0, func * 4, func);
	return 1;
}

stock Callback_Release(const input[E_CALLBACK_DATA])
{
	free(input[E_CALLBACK_DATA_ALLOC]);
}

stock Callback_Call(const result[E_CALLBACK_DATA], GLOBAL_TAG_TYPES:...)
{
	// Call the function with the given data.  We need some serious stack
	// manipulation skills in here to make it all work.
	if (result[E_CALLBACK_DATA_FLAGS] & e_CALLBACK_FLAGS_PUBLIC)
	{
		// I think I've got an even better way.
		new
			pointer = result[E_CALLBACK_DATA_POINTER];
		// Destroy one parameter.
		#emit LOAD.S.pri 8
		#emit ADD.C      0xFFFFFFFC
		#emit STOR.S.pri 12
		// Move the return address.
		#emit LOAD.S.pri 4
		#emit STOR.S.pri 8
		// Move the frame.
		#emit LOAD.S.pri 0
		#emit STOR.S.pri 4
		// Get the jump address.
		#emit LOAD.S.pri pointer
		#emit ADD.C      4
		#emit MOVE.alt
		// Mangle the stack.
		#emit LCTRL      5
		#emit ADD.C      4
		#emit SCTRL      4
		#emit SCTRL      5
		#emit MOVE.pri
		#emit SCTRL      6
		// Will never be called.
		#emit RETN
		// I just realised I don't know how to do this without type specifiers.
		// SOLVED!
		// Decompress the format.  Don't need "specifier" any more as we're not
		// using "CallRemoteFunction".  It's a shame as I had some nice code.
		new
			//specifier[32],
			type = ~result[E_CALLBACK_DATA_FORMAT],
			mask = 0x80000000,
			tmp,
			opp;
		/*while (type & mask)
		{
			mask >>>= 1;
		}*/
		if (type)
		{
			// Loop through parameters in reverse order.
			//tmp = Cell_GetLowestBit(mask);
			// Yes, this line is correct.
			//mask = type & ~-mask;
			//opp = tmp + 1;
			//if (opp == numargs())
			//{
				/*if (type == 1)
				{
					// No parameters.
					// Very easy, very quick!
					#emit PUSH.C     0
					#emit LCTRL      6
					#emit ADD.C      28
					#emit PUSH.pri
					#emit LOAD.S.pri opp
					#emit SCTRL      6
				}
				else
				{*/
			while (type != 1) //--tmp)
			{
					if (mask & 1)
					{
						//specifier[tmp] = 's';
					}
					else
					{
						//specifier[tmp] = 'i';
					}
					mask >>>= 1;
				}
				opp = result[E_CALLBACK_DATA_POINTER];
				tmp *= 4;
				#emit PUSH.S     tmp
				#emit LCTRL      6
				#emit ADD.C      28
				#emit PUSH.pri
				#emit LOAD.S.pri opp
				#emit SCTRL      6
			}
			P:C(else P:E("Insufficient parameters for public callback"););
		}
		P:C(else P:E("No format given for public callback"););
	}
	else
	{
		new
			size = _:result[E_CALLBACK_DATA_FLAGS],
			num  = 0,
			stack,
			mask = 0x80000000,
			addr,
			tmp,
			ininline = YSI_g_sInInline;
		YSI_g_sInInline = result[E_CALLBACK_DATA_POINTER];
		size = ((size & 0xFF) + (size >>> 8) + 3) * 4;
		#emit LCTRL      4
		#emit STOR.S.pri stack
		#emit LOAD.S.alt size
		#emit SUB
		#emit STOR.S.pri addr
		// Add more data for additional parameters.
		#emit SCTRL      4
		//#emit LCTRL      4
		size = result[E_CALLBACK_DATA_FORMAT];
		#emit LCTRL      5
		#emit ADD.C      16
		#emit STOR.S.pri tmp
		// OK, now the fun bit!
		while (size & mask)
		{
			mask >>>= 1;
		}
		mask >>>= 1;
		while (mask)
		{
			if (size & mask)
			{
				num += YSI_MAX_STRING;
				addr -= YSI_MAX_STRING * 4;
				#emit LOAD.S.pri addr
				#emit SCTRL      4
				// Copy the data.
				#emit PUSH.C     130
				#emit PUSH.C     520
				#emit PUSH.C     0
				#emit LREF.S.pri tmp
				#emit PUSH.pri
				#emit PUSH.S     addr
				#emit PUSH.C     20
				#emit SYSREQ.C   memcpy
				#emit STACK      24
			}
			else
			{
				num += 1;
				addr -= 1 * 4;
				#emit LREF.S.pri tmp
				#emit LOAD.I
				#emit PUSH.pri
			}
			mask >>>= 1;
			tmp += 4;
		}
		#emit LCTRL      5
		#emit STOR.S.pri tmp
		num *= 4;
		addr += num;
		// "addr" now contains the params stack address, "stack" contains the
		// starting stack address.  This code technically pushes an incorrect
		// destination size (it's 4x too big), but as the bytes to copy is
		// smaller this is not important.
		// Set the frame pointer.
		size = _:result[E_CALLBACK_DATA_FLAGS];
		#emit LOAD.S.pri size
		#emit SHR.C.pri  8
		#emit SHL.C.pri  2 // NOT SHR 6
		#emit LOAD.S.alt addr
		#emit ADD
		#emit STOR.S.pri tmp
		// Copy the data.
		size = ((size & 0xFF) + (size >>> 8) + 3) * 4;
		num = _:result[E_CALLBACK_DATA_ALLOC];
		#emit LOAD.S.pri size
		#emit PUSH.pri
		#emit PUSH.pri
		#emit PUSH.C     0
		#emit CONST.alt  YSI_gMallocMemory
		#emit LOAD.S.pri num
		#emit IDXADDR
		#emit PUSH.pri
		#emit PUSH.S     addr
		#emit PUSH.C     20
		#emit SYSREQ.C   memcpy
		#emit STACK      24
		// Store the return frame.
		#emit LOAD.S.alt tmp
		#emit LCTRL      5
		#emit STOR.I
		#emit MOVE.pri
		#emit ADD.C      4
		#emit MOVE.alt
		// Get the return address and call the function.
		#emit LCTRL      6
		#emit ADD.C      48              // 8
		#emit STOR.I                     // 12
		#emit LOAD.alt   YSI_g_sInInline // 20
		#emit LOAD.S.pri tmp             // 28
		#emit SCTRL      5               // 36
		#emit MOVE.pri                   // 40
		#emit SCTRL      6               // 48
		// Restore the stack.
		#emit LOAD.S.pri stack
		#emit SCTRL      4
		YSI_g_sInInline = ininline;
	}
}

// HOPEFULLY will derive the compressed format specifier for a function, with
// anything not "s" zero.
//#define _S<%0> (-1&_:@Rx:@Ry:@Rw:@Rv:@Ru:(0,%0,0))

#define _S<%0> (-1&_:~@Rx:@Ry:@Rv:@Ru:@Rw:(1,%0))

#define @Rx:@Ry:@Rv:@Ru:@Rw:(%9,s%0) @Rx:@Ry:@Rv:@Ru:@Rw:((%9)<<1,%0)
#define @Ry:@Rv:@Ru:@Rw:(%9,i%0) @Rx:@Ry:@Rv:@Ru:@Rw:((%9)<<1|1,%0)
#define @Rv:@Ru:@Rw:(%9,d%0) @Rx:@Ry:@Rv:@Ru:@Rw:((%9)<<1|1,%0)
#define @Ru:@Rw:(%9,f%0) @Rx:@Ry:@Rv:@Ru:@Rw:((%9)<<1|1,%0)
#define @Rw:(%9,) (%9)

/*#define @Ru:(%0i,%1) ~(1<<%1)&@Rx:@Ry:@Rw:@Rv:@Ru:(%0,%1+1)
#define @Rv:@Ru:(%0d,%1) ~(1<<%1)&@Rx:@Ry:@Rw:@Rv:@Ru:(%0,%1+1)
#define @Rw:@Rv:@Ru:(%0f,%1) ~(1<<%1)&@Rx:@Ry:@Rw:@Rv:@Ru:(%0,%1+1)
#define @Rx:@Ry:@Rw:@Rv:@Ru:(%0s,%1) @Rx:@Ry:@Rw:@Rv:@Ru:(%0,%1+1)
#define @Ry:@Rw:@Rv:@Ru:(,%1) ~(1<<%1)*/
















