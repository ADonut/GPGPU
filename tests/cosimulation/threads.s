# 
# Copyright (C) 2014 Jeff Bush
# 
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
# 
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
# 
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the
# Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
# Boston, MA  02110-1301, USA.
# 


#
# Basic multithreading tests
#

.globl _start
_start:  	getcr s0, 0		# Current thread ID
			btrue s0, skip_init

			move s1, -1
			setcr s1, 30	# Start all threads

skip_init:	shl s1, s0, 5
			setcr s0, 29
done:   	goto done