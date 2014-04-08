// 
// Copyright 2011-2012 Jeff Bush
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 

`include "defines.v"

//
// This module contains the control registers, special purpose locations
// that are used for system level functions like obtaining the current strand
// ID.
//

module control_registers
	#(parameter CORE_ID = 0)
	
	(input                                clk, 
	input                                 reset,
	
	// Control signals to/from other units
	output logic[`STRANDS_PER_CORE - 1:0] cr_strand_enable,
	output logic[31:0]                    cr_exception_handler_address,
	input                                 wb_latch_fault,
	input [31:0]                          wb_fault_pc,
	input [`STRAND_INDEX_WIDTH - 1:0]     wb_fault_strand,
	output logic                          cr_update_itlb_va_en,
	output logic                          cr_update_itlb_pa_en,
	output logic                          cr_update_dtlb_va_en,
	output logic                          cr_update_dtlb_pa_en,
	output[`TLB_INDEX_BITS:0]             cr_update_tlb_index,
	output[31:0]                          cr_update_tlb_value,

	// From memory access stage
	input[`STRAND_INDEX_WIDTH - 1:0]      ex_strand,	// strand that is reading or writing control register
	input control_register_t              ma_cr_index,
	input                                 ma_cr_read_en,
	input                                 ma_cr_write_en,
	input[31:0]                           ma_cr_write_value,
	
	// To writeback stage
	output logic[31:0]                    cr_read_value);

	logic[31:0] saved_fault_pc[0:3];
	logic[`TLB_INDEX_BITS:0] tlb_index;	// Note extra bit to indicate I or D cache (D cache is 1)

	// Need to move this out of always for Xilinx tools.
	wire[31:0] strand_saved_fault_pc = saved_fault_pc[ex_strand];

	// Transfer from control register
	always_comb
	begin
		unique case (ma_cr_index)
			CR_STRAND_ID: cr_read_value = { CORE_ID, ex_strand }; 		// Strand ID
			CR_EXCEPTION_HANDLER: cr_read_value = cr_exception_handler_address;
			CR_FAULT_ADDRESS: cr_read_value = strand_saved_fault_pc;
			CR_STRAND_ENABLE: cr_read_value = cr_strand_enable;
			default: cr_read_value = 0;
		endcase
	end

	// TLB update control
	assign cr_update_tlb_index = tlb_index[`TLB_INDEX_BITS - 1:0];
	assign cr_update_tlb_value = ma_cr_write_value;

	always_comb
	begin
		cr_update_itlb_va_en = 0;
		cr_update_itlb_pa_en = 0;
		cr_update_dtlb_va_en = 0;
		cr_update_dtlb_pa_en = 0;
		
		case (ma_cr_index)
			CR_UPDATE_TLB_VA:
				if (tlb_index[`TLB_INDEX_BITS])
					cr_update_itlb_va_en = 1;
				else
					cr_update_dtlb_va_en = 1;
			
			CR_UPDATE_TLB_PA:
				if (tlb_index[`TLB_INDEX_BITS])
					cr_update_itlb_pa_en = 1;
				else
					cr_update_dtlb_pa_en = 1;
		endcase
	end

	always_ff @(posedge clk, posedge reset)
	begin : update
		if (reset)
		begin
		 	cr_strand_enable <= 1'b1;	// Enable strand 0
			for (int i = 0; i < 4; i++)
				saved_fault_pc[i] <= 0;

			/*AUTORESET*/
			// Beginning of autoreset for uninitialized flops
			cr_exception_handler_address <= 32'h0;
			tlb_index <= {(1+(`TLB_INDEX_BITS)){1'b0}};
			// End of automatics
		end
		else
		begin
			assert($onehot0({ma_cr_read_en, ma_cr_write_en}));
		
			// Transfer to a control register
			if (ma_cr_write_en)
			begin
				case (ma_cr_index)
					CR_UPDATE_TLB_INDEX: tlb_index <= ma_cr_write_value;
					CR_HALT_STRAND: cr_strand_enable <= cr_strand_enable & ~(1'b1 << ex_strand);
					CR_EXCEPTION_HANDLER: cr_exception_handler_address <= ma_cr_write_value;
					CR_STRAND_ENABLE: cr_strand_enable <= ma_cr_write_value;
					CR_HALT: cr_strand_enable <= 0;	// HALT
				endcase
			end
			
			// Fault handling
			if (wb_latch_fault)
				saved_fault_pc[wb_fault_strand] <= wb_fault_pc;
		end
	end
endmodule

// Local Variables:
// verilog-typedef-regexp:"_t$"
// End:

