#include "ibm.h"

#include "x86.h"
#include "x86_flags.h"
#include "386_common.h"
#include "codegen.h"
#include "codegen_ir.h"
#include "codegen_ops.h"
#include "codegen_ops_arith.h"

uint32_t ropADD_AL_imm(codeblock_t *block, ir_data_t *ir, uint8_t opcode, uint32_t fetchdat, uint32_t op_32, uint32_t op_pc)
{
        uint8_t imm_data = fastreadb(cs + op_pc);

        uop_MOVZX(ir, IREG_flags_op1, IREG_AL);
        uop_ADD_IMM(ir, IREG_AL, IREG_AL, imm_data);
        uop_MOV_IMM(ir, IREG_flags_op2, imm_data);
        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ADD8);
        uop_MOVZX(ir, IREG_flags_res, IREG_AL);

        codegen_flags_changed = 1;
        return op_pc + 1;
}
uint32_t ropADD_AX_imm(codeblock_t *block, ir_data_t *ir, uint8_t opcode, uint32_t fetchdat, uint32_t op_32, uint32_t op_pc)
{
        uint16_t imm_data = fastreadw(cs + op_pc);

        uop_MOVZX(ir, IREG_flags_op1, IREG_AX);
        uop_ADD_IMM(ir, IREG_AX, IREG_AX, imm_data);
        uop_MOV_IMM(ir, IREG_flags_op2, imm_data);
        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ADD16);
        uop_MOVZX(ir, IREG_flags_res, IREG_AX);

        codegen_flags_changed = 1;
        return op_pc + 2;
}
uint32_t ropADD_EAX_imm(codeblock_t *block, ir_data_t *ir, uint8_t opcode, uint32_t fetchdat, uint32_t op_32, uint32_t op_pc)
{
        fetchdat = fastreadl(cs + op_pc);

        uop_MOV(ir, IREG_flags_op1, IREG_EAX);
        uop_ADD_IMM(ir, IREG_EAX, IREG_EAX, fetchdat);
        uop_MOV_IMM(ir, IREG_flags_op2, fetchdat);
        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ADD32);
        uop_MOV(ir, IREG_flags_res, IREG_EAX);

        codegen_flags_changed = 1;
        return op_pc + 4;
}
uint32_t ropADD_b_rm(codeblock_t *block, ir_data_t *ir, uint8_t opcode, uint32_t fetchdat, uint32_t op_32, uint32_t op_pc)
{
        int dest_reg = (fetchdat >> 3) & 7;

        if ((fetchdat & 0xc0) == 0xc0)
        {
                int src_reg = fetchdat & 7;
                
                uop_MOVZX(ir, IREG_flags_op1, IREG_8(dest_reg));
                uop_MOVZX(ir, IREG_flags_op2, IREG_8(src_reg));
                uop_ADD(ir, IREG_8(dest_reg), IREG_8(dest_reg), IREG_8(src_reg));
        }
        else
        {
                x86seg *target_seg;

                uop_MOV_IMM(ir, IREG_oldpc, cpu_state.oldpc);
                target_seg = codegen_generate_ea(ir, op_ea_seg, fetchdat, op_ssegs, &op_pc, op_32);
                codegen_check_seg_read(block, ir, target_seg);
                uop_MEM_LOAD_REG(ir, IREG_temp0_B, ireg_seg_base(target_seg), IREG_eaaddr);
                uop_MOVZX(ir, IREG_flags_op1, IREG_8(dest_reg));
                uop_MOVZX(ir, IREG_flags_op2, IREG_temp0_B);
                uop_ADD(ir, IREG_8(dest_reg), IREG_8(dest_reg), IREG_temp0_B);
        }

        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ADD8);
        uop_MOVZX(ir, IREG_flags_res, IREG_8(dest_reg));

        codegen_flags_changed = 1;
        return op_pc + 1;
}
uint32_t ropADD_b_rmw(codeblock_t *block, ir_data_t *ir, uint8_t opcode, uint32_t fetchdat, uint32_t op_32, uint32_t op_pc)
{
        int src_reg = (fetchdat >> 3) & 7;

        if ((fetchdat & 0xc0) == 0xc0)
        {
                int dest_reg = fetchdat & 7;

                uop_MOVZX(ir, IREG_flags_op1, IREG_8(dest_reg));
                uop_ADD(ir, IREG_8(dest_reg), IREG_8(dest_reg), IREG_8(src_reg));
                uop_MOVZX(ir, IREG_flags_res, IREG_8(dest_reg));
        }
        else
        {
                x86seg *target_seg;

                uop_MOV_IMM(ir, IREG_oldpc, cpu_state.oldpc);
                target_seg = codegen_generate_ea(ir, op_ea_seg, fetchdat, op_ssegs, &op_pc, op_32);
                codegen_check_seg_write(block, ir, target_seg);

                uop_MEM_LOAD_REG(ir, IREG_temp0_B, ireg_seg_base(target_seg), IREG_eaaddr);
                uop_ADD(ir, IREG_temp1_B, IREG_temp0_B, IREG_8(src_reg));
                uop_MEM_STORE_REG(ir, ireg_seg_base(target_seg), IREG_eaaddr, IREG_temp1_B);
                uop_MOVZX(ir, IREG_flags_op1, IREG_temp0_B);
                uop_MOVZX(ir, IREG_flags_res, IREG_temp1_B);
        }
        uop_MOVZX(ir, IREG_flags_op2, IREG_8(src_reg));
        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ADD8);

        codegen_flags_changed = 1;
        return op_pc + 1;
}
uint32_t ropADD_w_rm(codeblock_t *block, ir_data_t *ir, uint8_t opcode, uint32_t fetchdat, uint32_t op_32, uint32_t op_pc)
{
        int dest_reg = (fetchdat >> 3) & 7;

        if ((fetchdat & 0xc0) == 0xc0)
        {
                int src_reg = fetchdat & 7;

                uop_MOVZX(ir, IREG_flags_op1, IREG_16(dest_reg));
                uop_MOVZX(ir, IREG_flags_op2, IREG_16(src_reg));
                uop_ADD(ir, IREG_16(dest_reg), IREG_16(dest_reg), IREG_16(src_reg));
        }
        else
        {
                x86seg *target_seg;

                uop_MOV_IMM(ir, IREG_oldpc, cpu_state.oldpc);
                target_seg = codegen_generate_ea(ir, op_ea_seg, fetchdat, op_ssegs, &op_pc, op_32);
                codegen_check_seg_read(block, ir, target_seg);
                uop_MEM_LOAD_REG(ir, IREG_temp0_W, ireg_seg_base(target_seg), IREG_eaaddr);
                uop_MOVZX(ir, IREG_flags_op1, IREG_16(dest_reg));
                uop_MOVZX(ir, IREG_flags_op2, IREG_temp0_W);
                uop_ADD(ir, IREG_16(dest_reg), IREG_16(dest_reg), IREG_temp0_W);
        }

        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ADD16);
        uop_MOVZX(ir, IREG_flags_res, IREG_16(dest_reg));

        codegen_flags_changed = 1;
        return op_pc + 1;
}
uint32_t ropADD_w_rmw(codeblock_t *block, ir_data_t *ir, uint8_t opcode, uint32_t fetchdat, uint32_t op_32, uint32_t op_pc)
{
        int src_reg = (fetchdat >> 3) & 7;

        if ((fetchdat & 0xc0) == 0xc0)
        {
                int dest_reg = fetchdat & 7;

                uop_MOVZX(ir, IREG_flags_op1, IREG_16(dest_reg));
                uop_ADD(ir, IREG_16(dest_reg), IREG_16(dest_reg), IREG_16(src_reg));
                uop_MOVZX(ir, IREG_flags_res, IREG_16(dest_reg));
        }
        else
        {
                x86seg *target_seg;

                uop_MOV_IMM(ir, IREG_oldpc, cpu_state.oldpc);
                target_seg = codegen_generate_ea(ir, op_ea_seg, fetchdat, op_ssegs, &op_pc, op_32);
                codegen_check_seg_write(block, ir, target_seg);

                uop_MEM_LOAD_REG(ir, IREG_temp0_W, ireg_seg_base(target_seg), IREG_eaaddr);
                uop_ADD(ir, IREG_temp1_W, IREG_temp0_W, IREG_16(src_reg));
                uop_MEM_STORE_REG(ir, ireg_seg_base(target_seg), IREG_eaaddr, IREG_temp1_W);
                uop_MOVZX(ir, IREG_flags_op1, IREG_temp0_W);
                uop_MOVZX(ir, IREG_flags_res, IREG_temp1_W);
        }
        uop_MOVZX(ir, IREG_flags_op2, IREG_16(src_reg));
        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ADD16);

        codegen_flags_changed = 1;
        return op_pc + 1;
}
uint32_t ropADD_l_rm(codeblock_t *block, ir_data_t *ir, uint8_t opcode, uint32_t fetchdat, uint32_t op_32, uint32_t op_pc)
{
        int dest_reg = (fetchdat >> 3) & 7;

        if ((fetchdat & 0xc0) == 0xc0)
        {
                int src_reg = fetchdat & 7;

                uop_MOV(ir, IREG_flags_op1, IREG_32(dest_reg));
                uop_MOV(ir, IREG_flags_op2, IREG_32(src_reg));
                uop_ADD(ir, IREG_32(dest_reg), IREG_32(dest_reg), IREG_32(src_reg));
        }
        else
        {
                x86seg *target_seg;

                uop_MOV_IMM(ir, IREG_oldpc, cpu_state.oldpc);
                target_seg = codegen_generate_ea(ir, op_ea_seg, fetchdat, op_ssegs, &op_pc, op_32);
                codegen_check_seg_read(block, ir, target_seg);
                uop_MEM_LOAD_REG(ir, IREG_flags_op2, ireg_seg_base(target_seg), IREG_eaaddr);
                uop_MOV(ir, IREG_flags_op1, IREG_32(dest_reg));
                uop_ADD(ir, IREG_32(dest_reg), IREG_32(dest_reg), IREG_flags_op2);
        }

        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ADD32);
        uop_MOV(ir, IREG_flags_res, IREG_32(dest_reg));

        codegen_flags_changed = 1;
        return op_pc + 1;
}
uint32_t ropADD_l_rmw(codeblock_t *block, ir_data_t *ir, uint8_t opcode, uint32_t fetchdat, uint32_t op_32, uint32_t op_pc)
{
        int src_reg = (fetchdat >> 3) & 7;

        if ((fetchdat & 0xc0) == 0xc0)
        {
                int dest_reg = fetchdat & 7;

                uop_MOV(ir, IREG_flags_op1, IREG_32(dest_reg));
                uop_ADD(ir, IREG_32(dest_reg), IREG_32(dest_reg), IREG_32(src_reg));
                uop_MOV(ir, IREG_flags_res, IREG_32(dest_reg));
        }
        else
        {
                x86seg *target_seg;

                uop_MOV_IMM(ir, IREG_oldpc, cpu_state.oldpc);
                target_seg = codegen_generate_ea(ir, op_ea_seg, fetchdat, op_ssegs, &op_pc, op_32);
                codegen_check_seg_write(block, ir, target_seg);

                uop_MEM_LOAD_REG(ir, IREG_temp0, ireg_seg_base(target_seg), IREG_eaaddr);
                uop_ADD(ir, IREG_temp1, IREG_temp0, IREG_32(src_reg));
                uop_MEM_STORE_REG(ir, ireg_seg_base(target_seg), IREG_eaaddr, IREG_temp1);
                uop_MOV(ir, IREG_flags_op1, IREG_temp0);
                uop_MOV(ir, IREG_flags_res, IREG_temp1);
        }
        uop_MOV(ir, IREG_flags_op2, IREG_32(src_reg));
        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ADD32);

        codegen_flags_changed = 1;
        return op_pc + 1;
}

uint32_t ropCMP_AL_imm(codeblock_t *block, ir_data_t *ir, uint8_t opcode, uint32_t fetchdat, uint32_t op_32, uint32_t op_pc)
{
        uint8_t imm_data = fastreadb(cs + op_pc);

        uop_MOVZX(ir, IREG_flags_op1, IREG_AL);
        uop_SUB_IMM(ir, IREG_flags_res_B, IREG_AL, imm_data);
        uop_MOVZX(ir, IREG_flags_res, IREG_flags_res_B);
        uop_MOV_IMM(ir, IREG_flags_op2, imm_data);
        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_SUB8);

        codegen_flags_changed = 1;
        return op_pc + 1;
}
uint32_t ropCMP_AX_imm(codeblock_t *block, ir_data_t *ir, uint8_t opcode, uint32_t fetchdat, uint32_t op_32, uint32_t op_pc)
{
        uint16_t imm_data = fastreadw(cs + op_pc);

        uop_MOVZX(ir, IREG_flags_op1, IREG_AX);
        uop_SUB_IMM(ir, IREG_flags_res_W, IREG_AX, imm_data);
        uop_MOVZX(ir, IREG_flags_res, IREG_flags_res_W);
        uop_MOV_IMM(ir, IREG_flags_op2, imm_data);
        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_SUB16);

        codegen_flags_changed = 1;
        return op_pc + 2;
}
uint32_t ropCMP_EAX_imm(codeblock_t *block, ir_data_t *ir, uint8_t opcode, uint32_t fetchdat, uint32_t op_32, uint32_t op_pc)
{
        fetchdat = fastreadl(cs + op_pc);

        uop_MOV(ir, IREG_flags_op1, IREG_EAX);
        uop_SUB_IMM(ir, IREG_flags_res, IREG_EAX, fetchdat);
        uop_MOV_IMM(ir, IREG_flags_op2, fetchdat);
        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_SUB32);

        codegen_flags_changed = 1;
        return op_pc + 4;
}
uint32_t ropCMP_b_rm(codeblock_t *block, ir_data_t *ir, uint8_t opcode, uint32_t fetchdat, uint32_t op_32, uint32_t op_pc)
{
        int dest_reg = (fetchdat >> 3) & 7;

        if ((fetchdat & 0xc0) == 0xc0)
        {
                int src_reg = fetchdat & 7;

                uop_MOVZX(ir, IREG_flags_op1, IREG_8(dest_reg));
                uop_MOVZX(ir, IREG_flags_op2, IREG_8(src_reg));
                uop_SUB(ir, IREG_flags_res_B, IREG_8(dest_reg), IREG_8(src_reg));
        }
        else
        {
                x86seg *target_seg;

                uop_MOV_IMM(ir, IREG_oldpc, cpu_state.oldpc);
                target_seg = codegen_generate_ea(ir, op_ea_seg, fetchdat, op_ssegs, &op_pc, op_32);
                codegen_check_seg_read(block, ir, target_seg);
                uop_MEM_LOAD_REG(ir, IREG_temp0_B, ireg_seg_base(target_seg), IREG_eaaddr);
                uop_MOVZX(ir, IREG_flags_op1, IREG_8(dest_reg));
                uop_MOVZX(ir, IREG_flags_op2, IREG_temp0_B);
                uop_SUB(ir, IREG_flags_res_B, IREG_8(dest_reg), IREG_temp0_B);
        }

        uop_MOVZX(ir, IREG_flags_res, IREG_flags_res_B);
        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_SUB8);

        codegen_flags_changed = 1;
        return op_pc + 1;
}
uint32_t ropCMP_b_rmw(codeblock_t *block, ir_data_t *ir, uint8_t opcode, uint32_t fetchdat, uint32_t op_32, uint32_t op_pc)
{
        int src_reg = (fetchdat >> 3) & 7;

        if ((fetchdat & 0xc0) == 0xc0)
        {
                int dest_reg = fetchdat & 7;

                uop_MOVZX(ir, IREG_flags_op1, IREG_8(dest_reg));
                uop_MOVZX(ir, IREG_flags_op2, IREG_8(src_reg));
                uop_SUB(ir, IREG_flags_res_B, IREG_8(dest_reg), IREG_8(src_reg));
        }
        else
        {
                x86seg *target_seg;

                uop_MOV_IMM(ir, IREG_oldpc, cpu_state.oldpc);
                target_seg = codegen_generate_ea(ir, op_ea_seg, fetchdat, op_ssegs, &op_pc, op_32);
                codegen_check_seg_read(block, ir, target_seg);
                uop_MEM_LOAD_REG(ir, IREG_temp0_B, ireg_seg_base(target_seg), IREG_eaaddr);
                uop_MOVZX(ir, IREG_flags_op1, IREG_temp0_B);
                uop_MOVZX(ir, IREG_flags_op2, IREG_8(src_reg));
                uop_SUB(ir, IREG_flags_res_B, IREG_temp0_B, IREG_8(src_reg));
        }

        uop_MOVZX(ir, IREG_flags_res, IREG_flags_res_B);
        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_SUB8);

        codegen_flags_changed = 1;
        return op_pc + 1;
}
uint32_t ropCMP_w_rm(codeblock_t *block, ir_data_t *ir, uint8_t opcode, uint32_t fetchdat, uint32_t op_32, uint32_t op_pc)
{
        int dest_reg = (fetchdat >> 3) & 7;

        if ((fetchdat & 0xc0) == 0xc0)
        {
                int src_reg = fetchdat & 7;

                uop_MOVZX(ir, IREG_flags_op1, IREG_16(dest_reg));
                uop_MOVZX(ir, IREG_flags_op2, IREG_16(src_reg));
                uop_SUB(ir, IREG_flags_res_W, IREG_16(dest_reg), IREG_16(src_reg));
        }
        else
        {
                x86seg *target_seg;

                uop_MOV_IMM(ir, IREG_oldpc, cpu_state.oldpc);
                target_seg = codegen_generate_ea(ir, op_ea_seg, fetchdat, op_ssegs, &op_pc, op_32);
                codegen_check_seg_read(block, ir, target_seg);
                uop_MEM_LOAD_REG(ir, IREG_temp0_W, ireg_seg_base(target_seg), IREG_eaaddr);
                uop_MOVZX(ir, IREG_flags_op1, IREG_16(dest_reg));
                uop_MOVZX(ir, IREG_flags_op2, IREG_temp0_W);
                uop_SUB(ir, IREG_flags_res_W, IREG_16(dest_reg), IREG_temp0_W);
        }

        uop_MOVZX(ir, IREG_flags_res, IREG_flags_res_W);
        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_SUB16);

        codegen_flags_changed = 1;
        return op_pc + 1;
}
uint32_t ropCMP_w_rmw(codeblock_t *block, ir_data_t *ir, uint8_t opcode, uint32_t fetchdat, uint32_t op_32, uint32_t op_pc)
{
        int src_reg = (fetchdat >> 3) & 7;

        if ((fetchdat & 0xc0) == 0xc0)
        {
                int dest_reg = fetchdat & 7;

                uop_MOVZX(ir, IREG_flags_op1, IREG_16(dest_reg));
                uop_MOVZX(ir, IREG_flags_op2, IREG_16(src_reg));
                uop_SUB(ir, IREG_flags_res_W, IREG_16(dest_reg), IREG_16(src_reg));
        }
        else
        {
                x86seg *target_seg;

                uop_MOV_IMM(ir, IREG_oldpc, cpu_state.oldpc);
                target_seg = codegen_generate_ea(ir, op_ea_seg, fetchdat, op_ssegs, &op_pc, op_32);
                codegen_check_seg_read(block, ir, target_seg);
                uop_MEM_LOAD_REG(ir, IREG_temp0_W, ireg_seg_base(target_seg), IREG_eaaddr);
                uop_MOVZX(ir, IREG_flags_op1, IREG_temp0_W);
                uop_MOVZX(ir, IREG_flags_op2, IREG_16(src_reg));
                uop_SUB(ir, IREG_flags_res_W, IREG_temp0_W, IREG_16(src_reg));
        }

        uop_MOVZX(ir, IREG_flags_res, IREG_flags_res_W);
        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_SUB16);

        codegen_flags_changed = 1;
        return op_pc + 1;
}
uint32_t ropCMP_l_rm(codeblock_t *block, ir_data_t *ir, uint8_t opcode, uint32_t fetchdat, uint32_t op_32, uint32_t op_pc)
{
        int dest_reg = (fetchdat >> 3) & 7;

        if ((fetchdat & 0xc0) == 0xc0)
        {
                int src_reg = fetchdat & 7;

                uop_MOV(ir, IREG_flags_op1, IREG_32(dest_reg));
                uop_MOV(ir, IREG_flags_op2, IREG_32(src_reg));
                uop_SUB(ir, IREG_flags_res, IREG_32(dest_reg), IREG_32(src_reg));
        }
        else
        {
                x86seg *target_seg;

                uop_MOV_IMM(ir, IREG_oldpc, cpu_state.oldpc);
                target_seg = codegen_generate_ea(ir, op_ea_seg, fetchdat, op_ssegs, &op_pc, op_32);
                codegen_check_seg_read(block, ir, target_seg);
                uop_MEM_LOAD_REG(ir, IREG_temp0, ireg_seg_base(target_seg), IREG_eaaddr);
                uop_MOV(ir, IREG_flags_op1, IREG_32(dest_reg));
                uop_MOV(ir, IREG_flags_op2, IREG_temp0);
                uop_SUB(ir, IREG_flags_res, IREG_32(dest_reg), IREG_temp0);
        }

        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_SUB32);

        codegen_flags_changed = 1;
        return op_pc + 1;
}
uint32_t ropCMP_l_rmw(codeblock_t *block, ir_data_t *ir, uint8_t opcode, uint32_t fetchdat, uint32_t op_32, uint32_t op_pc)
{
        int src_reg = (fetchdat >> 3) & 7;

        if ((fetchdat & 0xc0) == 0xc0)
        {
                int dest_reg = fetchdat & 7;

                uop_MOV(ir, IREG_flags_op1, IREG_32(dest_reg));
                uop_MOV(ir, IREG_flags_op2, IREG_32(src_reg));
                uop_SUB(ir, IREG_flags_res, IREG_32(dest_reg), IREG_32(src_reg));
        }
        else
        {
                x86seg *target_seg;

                uop_MOV_IMM(ir, IREG_oldpc, cpu_state.oldpc);
                target_seg = codegen_generate_ea(ir, op_ea_seg, fetchdat, op_ssegs, &op_pc, op_32);
                codegen_check_seg_read(block, ir, target_seg);
                uop_MEM_LOAD_REG(ir, IREG_temp0, ireg_seg_base(target_seg), IREG_eaaddr);
                uop_MOV(ir, IREG_flags_op1, IREG_temp0);
                uop_MOV(ir, IREG_flags_op2, IREG_32(src_reg));
                uop_SUB(ir, IREG_flags_res, IREG_temp0, IREG_32(src_reg));
        }

        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_SUB32);

        codegen_flags_changed = 1;
        return op_pc + 1;
}

uint32_t ropSUB_AL_imm(codeblock_t *block, ir_data_t *ir, uint8_t opcode, uint32_t fetchdat, uint32_t op_32, uint32_t op_pc)
{
        uint8_t imm_data = fastreadb(cs + op_pc);

        uop_MOVZX(ir, IREG_flags_op1, IREG_AL);
        uop_SUB_IMM(ir, IREG_AL, IREG_AL, imm_data);
        uop_MOV_IMM(ir, IREG_flags_op2, imm_data);
        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_SUB8);
        uop_MOVZX(ir, IREG_flags_res, IREG_AL);

        codegen_flags_changed = 1;
        return op_pc + 1;
}
uint32_t ropSUB_AX_imm(codeblock_t *block, ir_data_t *ir, uint8_t opcode, uint32_t fetchdat, uint32_t op_32, uint32_t op_pc)
{
        uint16_t imm_data = fastreadw(cs + op_pc);

        uop_MOVZX(ir, IREG_flags_op1, IREG_AX);
        uop_SUB_IMM(ir, IREG_AX, IREG_AX, imm_data);
        uop_MOV_IMM(ir, IREG_flags_op2, imm_data);
        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_SUB16);
        uop_MOVZX(ir, IREG_flags_res, IREG_AX);

        codegen_flags_changed = 1;
        return op_pc + 2;
}
uint32_t ropSUB_EAX_imm(codeblock_t *block, ir_data_t *ir, uint8_t opcode, uint32_t fetchdat, uint32_t op_32, uint32_t op_pc)
{
        fetchdat = fastreadl(cs + op_pc);

        uop_MOV(ir, IREG_flags_op1, IREG_EAX);
        uop_SUB_IMM(ir, IREG_EAX, IREG_EAX, fetchdat);
        uop_MOV_IMM(ir, IREG_flags_op2, fetchdat);
        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_SUB32);
        uop_MOV(ir, IREG_flags_res, IREG_EAX);

        codegen_flags_changed = 1;
        return op_pc + 4;
}
uint32_t ropSUB_b_rm(codeblock_t *block, ir_data_t *ir, uint8_t opcode, uint32_t fetchdat, uint32_t op_32, uint32_t op_pc)
{
        int dest_reg = (fetchdat >> 3) & 7;

        if ((fetchdat & 0xc0) == 0xc0)
        {
                int src_reg = fetchdat & 7;

                uop_MOVZX(ir, IREG_flags_op1, IREG_8(dest_reg));
                uop_MOVZX(ir, IREG_flags_op2, IREG_8(src_reg));
                uop_SUB(ir, IREG_8(dest_reg), IREG_8(dest_reg), IREG_8(src_reg));
        }
        else
        {
                x86seg *target_seg;

                uop_MOV_IMM(ir, IREG_oldpc, cpu_state.oldpc);
                target_seg = codegen_generate_ea(ir, op_ea_seg, fetchdat, op_ssegs, &op_pc, op_32);
                codegen_check_seg_read(block, ir, target_seg);
                uop_MEM_LOAD_REG(ir, IREG_temp0_B, ireg_seg_base(target_seg), IREG_eaaddr);
                uop_MOVZX(ir, IREG_flags_op1, IREG_8(dest_reg));
                uop_MOVZX(ir, IREG_flags_op2, IREG_temp0_B);
                uop_SUB(ir, IREG_8(dest_reg), IREG_8(dest_reg), IREG_temp0_B);
        }

        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_SUB8);
        uop_MOVZX(ir, IREG_flags_res, IREG_8(dest_reg));

        codegen_flags_changed = 1;
        return op_pc + 1;
}
uint32_t ropSUB_b_rmw(codeblock_t *block, ir_data_t *ir, uint8_t opcode, uint32_t fetchdat, uint32_t op_32, uint32_t op_pc)
{
        int src_reg = (fetchdat >> 3) & 7;

        if ((fetchdat & 0xc0) == 0xc0)
        {
                int dest_reg = fetchdat & 7;

                uop_MOVZX(ir, IREG_flags_op1, IREG_8(dest_reg));
                uop_SUB(ir, IREG_8(dest_reg), IREG_8(dest_reg), IREG_8(src_reg));
                uop_MOVZX(ir, IREG_flags_res, IREG_8(dest_reg));
        }
        else
        {
                x86seg *target_seg;

                uop_MOV_IMM(ir, IREG_oldpc, cpu_state.oldpc);
                target_seg = codegen_generate_ea(ir, op_ea_seg, fetchdat, op_ssegs, &op_pc, op_32);
                codegen_check_seg_write(block, ir, target_seg);

                uop_MEM_LOAD_REG(ir, IREG_temp0_B, ireg_seg_base(target_seg), IREG_eaaddr);
                uop_SUB(ir, IREG_temp1_B, IREG_temp0_B, IREG_8(src_reg));
                uop_MEM_STORE_REG(ir, ireg_seg_base(target_seg), IREG_eaaddr, IREG_temp1_B);
                uop_MOVZX(ir, IREG_flags_op1, IREG_temp0_B);
                uop_MOVZX(ir, IREG_flags_res, IREG_temp1_B);
        }
        uop_MOVZX(ir, IREG_flags_op2, IREG_8(src_reg));
        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_SUB8);

        codegen_flags_changed = 1;
        return op_pc + 1;
}
uint32_t ropSUB_w_rm(codeblock_t *block, ir_data_t *ir, uint8_t opcode, uint32_t fetchdat, uint32_t op_32, uint32_t op_pc)
{
        int dest_reg = (fetchdat >> 3) & 7;

        if ((fetchdat & 0xc0) == 0xc0)
        {
                int src_reg = fetchdat & 7;

                uop_MOVZX(ir, IREG_flags_op1, IREG_16(dest_reg));
                uop_MOVZX(ir, IREG_flags_op2, IREG_16(src_reg));
                uop_SUB(ir, IREG_16(dest_reg), IREG_16(dest_reg), IREG_16(src_reg));
        }
        else
        {
                x86seg *target_seg;

                uop_MOV_IMM(ir, IREG_oldpc, cpu_state.oldpc);
                target_seg = codegen_generate_ea(ir, op_ea_seg, fetchdat, op_ssegs, &op_pc, op_32);
                codegen_check_seg_read(block, ir, target_seg);
                uop_MEM_LOAD_REG(ir, IREG_temp0_W, ireg_seg_base(target_seg), IREG_eaaddr);
                uop_MOVZX(ir, IREG_flags_op1, IREG_16(dest_reg));
                uop_MOVZX(ir, IREG_flags_op2, IREG_temp0_W);
                uop_SUB(ir, IREG_16(dest_reg), IREG_16(dest_reg), IREG_temp0_W);
        }

        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_SUB16);
        uop_MOVZX(ir, IREG_flags_res, IREG_16(dest_reg));

        codegen_flags_changed = 1;
        return op_pc + 1;
}
uint32_t ropSUB_w_rmw(codeblock_t *block, ir_data_t *ir, uint8_t opcode, uint32_t fetchdat, uint32_t op_32, uint32_t op_pc)
{
        int src_reg = (fetchdat >> 3) & 7;

        if ((fetchdat & 0xc0) == 0xc0)
        {
                int dest_reg = fetchdat & 7;

                uop_MOVZX(ir, IREG_flags_op1, IREG_16(dest_reg));
                uop_SUB(ir, IREG_16(dest_reg), IREG_16(dest_reg), IREG_16(src_reg));
                uop_MOVZX(ir, IREG_flags_res, IREG_16(dest_reg));
        }
        else
        {
                x86seg *target_seg;

                uop_MOV_IMM(ir, IREG_oldpc, cpu_state.oldpc);
                target_seg = codegen_generate_ea(ir, op_ea_seg, fetchdat, op_ssegs, &op_pc, op_32);
                codegen_check_seg_write(block, ir, target_seg);

                uop_MEM_LOAD_REG(ir, IREG_temp0_W, ireg_seg_base(target_seg), IREG_eaaddr);
                uop_SUB(ir, IREG_temp1_W, IREG_temp0_W, IREG_16(src_reg));
                uop_MEM_STORE_REG(ir, ireg_seg_base(target_seg), IREG_eaaddr, IREG_temp1_W);
                uop_MOVZX(ir, IREG_flags_op1, IREG_temp0_W);
                uop_MOVZX(ir, IREG_flags_res, IREG_temp1_W);
        }
        uop_MOVZX(ir, IREG_flags_op2, IREG_16(src_reg));
        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_SUB16);

        codegen_flags_changed = 1;
        return op_pc + 1;
}
uint32_t ropSUB_l_rm(codeblock_t *block, ir_data_t *ir, uint8_t opcode, uint32_t fetchdat, uint32_t op_32, uint32_t op_pc)
{
        int dest_reg = (fetchdat >> 3) & 7;

        if ((fetchdat & 0xc0) == 0xc0)
        {
                int src_reg = fetchdat & 7;

                uop_MOV(ir, IREG_flags_op1, IREG_32(dest_reg));
                uop_MOV(ir, IREG_flags_op2, IREG_32(src_reg));
                uop_SUB(ir, IREG_32(dest_reg), IREG_32(dest_reg), IREG_32(src_reg));
        }
        else
        {
                x86seg *target_seg;

                uop_MOV_IMM(ir, IREG_oldpc, cpu_state.oldpc);
                target_seg = codegen_generate_ea(ir, op_ea_seg, fetchdat, op_ssegs, &op_pc, op_32);
                codegen_check_seg_read(block, ir, target_seg);
                uop_MEM_LOAD_REG(ir, IREG_flags_op2, ireg_seg_base(target_seg), IREG_eaaddr);
                uop_MOV(ir, IREG_flags_op1, IREG_32(dest_reg));
                uop_SUB(ir, IREG_32(dest_reg), IREG_32(dest_reg), IREG_flags_op2);
        }

        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_SUB32);
        uop_MOV(ir, IREG_flags_res, IREG_32(dest_reg));

        codegen_flags_changed = 1;
        return op_pc + 1;
}
uint32_t ropSUB_l_rmw(codeblock_t *block, ir_data_t *ir, uint8_t opcode, uint32_t fetchdat, uint32_t op_32, uint32_t op_pc)
{
        int src_reg = (fetchdat >> 3) & 7;

        if ((fetchdat & 0xc0) == 0xc0)
        {
                int dest_reg = fetchdat & 7;

                uop_MOV(ir, IREG_flags_op1, IREG_32(dest_reg));
                uop_SUB(ir, IREG_32(dest_reg), IREG_32(dest_reg), IREG_32(src_reg));
                uop_MOV(ir, IREG_flags_res, IREG_32(dest_reg));
        }
        else
        {
                x86seg *target_seg;

                uop_MOV_IMM(ir, IREG_oldpc, cpu_state.oldpc);
                target_seg = codegen_generate_ea(ir, op_ea_seg, fetchdat, op_ssegs, &op_pc, op_32);
                codegen_check_seg_write(block, ir, target_seg);

                uop_MEM_LOAD_REG(ir, IREG_temp0, ireg_seg_base(target_seg), IREG_eaaddr);
                uop_SUB(ir, IREG_temp1, IREG_temp0, IREG_32(src_reg));
                uop_MEM_STORE_REG(ir, ireg_seg_base(target_seg), IREG_eaaddr, IREG_temp1);
                uop_MOV(ir, IREG_flags_op1, IREG_temp0);
                uop_MOV(ir, IREG_flags_res, IREG_temp1);
        }
        uop_MOV(ir, IREG_flags_op2, IREG_32(src_reg));
        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_SUB32);

        codegen_flags_changed = 1;
        return op_pc + 1;
}

uint32_t rop80(codeblock_t *block, ir_data_t *ir, uint8_t opcode, uint32_t fetchdat, uint32_t op_32, uint32_t op_pc)
{
        if ((fetchdat & 0x30) == 0x10) /*ADC/SBB*/
                return 0;

        if ((fetchdat & 0xc0) == 0xc0)
        {
                int dest_reg = fetchdat & 7;
                uint8_t imm = fastreadb(cs + op_pc + 1);
                
                switch (fetchdat & 0x38)
                {
                        case 0x00: /*ADD*/
                        uop_MOVZX(ir, IREG_flags_op1, IREG_8(dest_reg));
                        uop_ADD_IMM(ir, IREG_8(dest_reg), IREG_8(dest_reg), imm);
                        uop_MOV_IMM(ir, IREG_flags_op2, imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ADD8);
                        uop_MOVZX(ir, IREG_flags_res, IREG_8(dest_reg));
                        break;

                        case 0x08: /*OR*/
                        uop_OR_IMM(ir, IREG_8(dest_reg), IREG_8(dest_reg), imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ZN8);
                        uop_MOVZX(ir, IREG_flags_res, IREG_8(dest_reg));
                        break;

                        case 0x20: /*AND*/
                        uop_AND_IMM(ir, IREG_8(dest_reg), IREG_8(dest_reg), imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ZN8);
                        uop_MOVZX(ir, IREG_flags_res, IREG_8(dest_reg));
                        break;

                        case 0x28: /*SUB*/
                        uop_MOVZX(ir, IREG_flags_op1, IREG_8(dest_reg));
                        uop_SUB_IMM(ir, IREG_8(dest_reg), IREG_8(dest_reg), imm);
                        uop_MOV_IMM(ir, IREG_flags_op2, imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_SUB8);
                        uop_MOVZX(ir, IREG_flags_res, IREG_8(dest_reg));
                        break;

                        case 0x30: /*XOR*/
                        uop_XOR_IMM(ir, IREG_8(dest_reg), IREG_8(dest_reg), imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ZN8);
                        uop_MOVZX(ir, IREG_flags_res, IREG_8(dest_reg));
                        break;

                        case 0x38: /*CMP*/
                        uop_MOVZX(ir, IREG_flags_op1, IREG_8(dest_reg));
                        uop_SUB_IMM(ir, IREG_flags_res_B, IREG_8(dest_reg), imm);
                        uop_MOVZX(ir, IREG_flags_res, IREG_flags_res_B);
                        uop_MOV_IMM(ir, IREG_flags_op2, imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_SUB8);
                        break;

                        default:
                        return 0;
                }
        }
        else
        {
                x86seg *target_seg;
                uint8_t imm;

                uop_MOV_IMM(ir, IREG_oldpc, cpu_state.oldpc);
                target_seg = codegen_generate_ea(ir, op_ea_seg, fetchdat, op_ssegs, &op_pc, op_32);
                if ((fetchdat & 0x38) == 0x38) /*CMP*/
                        codegen_check_seg_read(block, ir, target_seg);
                else
                        codegen_check_seg_write(block, ir, target_seg);
                imm = fastreadb(cs + op_pc + 1);
                uop_MEM_LOAD_REG(ir, IREG_temp0_B, ireg_seg_base(target_seg), IREG_eaaddr);

                switch (fetchdat & 0x38)
                {
                        case 0x00: /*ADD*/
                        uop_ADD_IMM(ir, IREG_temp1_B, IREG_temp0_B, imm);
                        uop_MEM_STORE_REG(ir, ireg_seg_base(target_seg), IREG_eaaddr, IREG_temp1_B);
                        uop_MOVZX(ir, IREG_flags_op1, IREG_temp0_B);
                        uop_MOV_IMM(ir, IREG_flags_op2, imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ADD8);
                        uop_MOVZX(ir, IREG_flags_res, IREG_temp1_B);
                        break;

                        case 0x08: /*OR*/
                        uop_OR_IMM(ir, IREG_temp0_B, IREG_temp0_B, imm);
                        uop_MEM_STORE_REG(ir, ireg_seg_base(target_seg), IREG_eaaddr, IREG_temp0_B);
                        uop_MOVZX(ir, IREG_flags_res, IREG_temp0_B);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ZN8);
                        break;

                        case 0x20: /*AND*/
                        uop_AND_IMM(ir, IREG_temp0_B, IREG_temp0_B, imm);
                        uop_MEM_STORE_REG(ir, ireg_seg_base(target_seg), IREG_eaaddr, IREG_temp0_B);
                        uop_MOVZX(ir, IREG_flags_res, IREG_temp0_B);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ZN8);
                        break;

                        case 0x28: /*SUB*/
                        uop_SUB_IMM(ir, IREG_temp1_B, IREG_temp0_B, imm);
                        uop_MEM_STORE_REG(ir, ireg_seg_base(target_seg), IREG_eaaddr, IREG_temp1_B);
                        uop_MOVZX(ir, IREG_flags_op1, IREG_temp0_B);
                        uop_MOV_IMM(ir, IREG_flags_op2, imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_SUB8);
                        uop_MOVZX(ir, IREG_flags_res, IREG_temp1_B);
                        break;

                        case 0x30: /*XOR*/
                        uop_XOR_IMM(ir, IREG_temp0_B, IREG_temp0_B, imm);
                        uop_MEM_STORE_REG(ir, ireg_seg_base(target_seg), IREG_eaaddr, IREG_temp0_B);
                        uop_MOVZX(ir, IREG_flags_res, IREG_temp0_B);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ZN8);
                        break;

                        case 0x38: /*CMP*/
                        uop_MOVZX(ir, IREG_flags_op1, IREG_temp0_B);
                        uop_MOV_IMM(ir, IREG_flags_op2, imm);
                        uop_SUB_IMM(ir, IREG_flags_res_B, IREG_temp0_B, imm);
                        uop_MOVZX(ir, IREG_flags_res, IREG_flags_res_B);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_SUB8);
                        break;

                        default:
                        return 0;
                }
        }

        codegen_flags_changed = 1;
        return op_pc + 2;
}
uint32_t rop81_w(codeblock_t *block, ir_data_t *ir, uint8_t opcode, uint32_t fetchdat, uint32_t op_32, uint32_t op_pc)
{
        if ((fetchdat & 0x30) == 0x10) /*ADC/SBB*/
                return 0;

        if ((fetchdat & 0xc0) == 0xc0)
        {
                int dest_reg = fetchdat & 7;
                uint16_t imm = fastreadw(cs + op_pc + 1);

                switch (fetchdat & 0x38)
                {
                        case 0x00: /*ADD*/
                        uop_MOVZX(ir, IREG_flags_op1, IREG_16(dest_reg));
                        uop_ADD_IMM(ir, IREG_16(dest_reg), IREG_16(dest_reg), imm);
                        uop_MOV_IMM(ir, IREG_flags_op2, imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ADD16);
                        uop_MOVZX(ir, IREG_flags_res, IREG_16(dest_reg));
                        break;

                        case 0x08: /*OR*/
                        uop_OR_IMM(ir, IREG_16(dest_reg), IREG_16(dest_reg), imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ZN16);
                        uop_MOVZX(ir, IREG_flags_res, IREG_16(dest_reg));
                        break;

                        case 0x20: /*AND*/
                        uop_AND_IMM(ir, IREG_16(dest_reg), IREG_16(dest_reg), imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ZN16);
                        uop_MOVZX(ir, IREG_flags_res, IREG_16(dest_reg));
                        break;

                        case 0x28: /*SUB*/
                        uop_MOVZX(ir, IREG_flags_op1, IREG_16(dest_reg));
                        uop_SUB_IMM(ir, IREG_16(dest_reg), IREG_16(dest_reg), imm);
                        uop_MOV_IMM(ir, IREG_flags_op2, imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_SUB16);
                        uop_MOVZX(ir, IREG_flags_res, IREG_16(dest_reg));
                        break;

                        case 0x30: /*XOR*/
                        uop_XOR_IMM(ir, IREG_16(dest_reg), IREG_16(dest_reg), imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ZN16);
                        uop_MOVZX(ir, IREG_flags_res, IREG_16(dest_reg));
                        break;

                        case 0x38: /*CMP*/
                        uop_MOVZX(ir, IREG_flags_op1, IREG_16(dest_reg));
                        uop_SUB_IMM(ir, IREG_flags_res_W, IREG_16(dest_reg), imm);
                        uop_MOVZX(ir, IREG_flags_res, IREG_flags_res_W);
                        uop_MOV_IMM(ir, IREG_flags_op2, imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_SUB16);
                        break;

                        default:
                        return 0;
                }
        }
        else
        {
                x86seg *target_seg;
                uint16_t imm;

                uop_MOV_IMM(ir, IREG_oldpc, cpu_state.oldpc);
                target_seg = codegen_generate_ea(ir, op_ea_seg, fetchdat, op_ssegs, &op_pc, op_32);
                if ((fetchdat & 0x38) == 0x38) /*CMP*/
                        codegen_check_seg_read(block, ir, target_seg);
                else
                        codegen_check_seg_write(block, ir, target_seg);
                imm = fastreadw(cs + op_pc + 1);
                uop_MEM_LOAD_REG(ir, IREG_temp0_W, ireg_seg_base(target_seg), IREG_eaaddr);

                switch (fetchdat & 0x38)
                {
                        case 0x00: /*ADD*/
                        uop_ADD_IMM(ir, IREG_temp1_W, IREG_temp0_W, imm);
                        uop_MEM_STORE_REG(ir, ireg_seg_base(target_seg), IREG_eaaddr, IREG_temp1_W);
                        uop_MOVZX(ir, IREG_flags_op1, IREG_temp0_W);
                        uop_MOV_IMM(ir, IREG_flags_op2, imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ADD16);
                        uop_MOVZX(ir, IREG_flags_res, IREG_temp1_W);
                        break;

                        case 0x08: /*OR*/
                        uop_OR_IMM(ir, IREG_temp0_W, IREG_temp0_W, imm);
                        uop_MEM_STORE_REG(ir, ireg_seg_base(target_seg), IREG_eaaddr, IREG_temp0_W);
                        uop_MOVZX(ir, IREG_flags_res, IREG_temp0_W);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ZN16);
                        break;

                        case 0x20: /*AND*/
                        uop_AND_IMM(ir, IREG_temp0_W, IREG_temp0_W, imm);
                        uop_MEM_STORE_REG(ir, ireg_seg_base(target_seg), IREG_eaaddr, IREG_temp0_W);
                        uop_MOVZX(ir, IREG_flags_res, IREG_temp0_W);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ZN16);
                        break;

                        case 0x28: /*SUB*/
                        uop_SUB_IMM(ir, IREG_temp1_W, IREG_temp0_W, imm);
                        uop_MEM_STORE_REG(ir, ireg_seg_base(target_seg), IREG_eaaddr, IREG_temp1_W);
                        uop_MOVZX(ir, IREG_flags_op1, IREG_temp0_W);
                        uop_MOV_IMM(ir, IREG_flags_op2, imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_SUB16);
                        uop_MOVZX(ir, IREG_flags_res, IREG_temp1_W);
                        break;

                        case 0x30: /*XOR*/
                        uop_XOR_IMM(ir, IREG_temp0_W, IREG_temp0_W, imm);
                        uop_MEM_STORE_REG(ir, ireg_seg_base(target_seg), IREG_eaaddr, IREG_temp0_W);
                        uop_MOVZX(ir, IREG_flags_res, IREG_temp0_W);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ZN16);
                        break;

                        case 0x38: /*CMP*/
                        uop_MOVZX(ir, IREG_flags_op1, IREG_temp0_W);
                        uop_MOV_IMM(ir, IREG_flags_op2, imm);
                        uop_SUB_IMM(ir, IREG_flags_res_W, IREG_temp0_W, imm);
                        uop_MOVZX(ir, IREG_flags_res, IREG_flags_res_W);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_SUB16);
                        break;

                        default:
                        return 0;
                }
        }

        codegen_flags_changed = 1;
        return op_pc + 3;
}
uint32_t rop81_l(codeblock_t *block, ir_data_t *ir, uint8_t opcode, uint32_t fetchdat, uint32_t op_32, uint32_t op_pc)
{
        if ((fetchdat & 0x30) == 0x10) /*ADC/SBB*/
                return 0;

        if ((fetchdat & 0xc0) == 0xc0)
        {
                int dest_reg = fetchdat & 7;
                uint32_t imm = fastreadl(cs + op_pc + 1);

                switch (fetchdat & 0x38)
                {
                        case 0x00: /*ADD*/
                        uop_MOV(ir, IREG_flags_op1, IREG_32(dest_reg));
                        uop_ADD_IMM(ir, IREG_32(dest_reg), IREG_32(dest_reg), imm);
                        uop_MOV_IMM(ir, IREG_flags_op2, imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ADD32);
                        uop_MOV(ir, IREG_flags_res, IREG_32(dest_reg));
                        break;

                        case 0x08: /*OR*/
                        uop_OR_IMM(ir, IREG_32(dest_reg), IREG_32(dest_reg), imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ZN32);
                        uop_MOV(ir, IREG_flags_res, IREG_32(dest_reg));
                        break;

                        case 0x20: /*AND*/
                        uop_AND_IMM(ir, IREG_32(dest_reg), IREG_32(dest_reg), imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ZN32);
                        uop_MOV(ir, IREG_flags_res, IREG_32(dest_reg));
                        break;

                        case 0x28: /*SUB*/
                        uop_MOV(ir, IREG_flags_op1, IREG_32(dest_reg));
                        uop_SUB_IMM(ir, IREG_32(dest_reg), IREG_32(dest_reg), imm);
                        uop_MOV_IMM(ir, IREG_flags_op2, imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_SUB32);
                        uop_MOV(ir, IREG_flags_res, IREG_32(dest_reg));
                        break;

                        case 0x30: /*XOR*/
                        uop_XOR_IMM(ir, IREG_32(dest_reg), IREG_32(dest_reg), imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ZN32);
                        uop_MOV(ir, IREG_flags_res, IREG_32(dest_reg));
                        break;

                        case 0x38: /*CMP*/
                        uop_MOV(ir, IREG_flags_op1, IREG_32(dest_reg));
                        uop_SUB_IMM(ir, IREG_flags_res, IREG_32(dest_reg), imm);
                        uop_MOV_IMM(ir, IREG_flags_op2, imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_SUB32);
                        break;

                        default:
                        return 0;
                }
        }
        else
        {
                x86seg *target_seg;
                uint32_t imm;

                uop_MOV_IMM(ir, IREG_oldpc, cpu_state.oldpc);
                target_seg = codegen_generate_ea(ir, op_ea_seg, fetchdat, op_ssegs, &op_pc, op_32);
                if ((fetchdat & 0x38) == 0x38) /*CMP*/
                        codegen_check_seg_read(block, ir, target_seg);
                else
                        codegen_check_seg_write(block, ir, target_seg);
                imm = fastreadl(cs + op_pc + 1);
                uop_MEM_LOAD_REG(ir, IREG_temp0, ireg_seg_base(target_seg), IREG_eaaddr);

                switch (fetchdat & 0x38)
                {
                        case 0x00: /*ADD*/
                        uop_ADD_IMM(ir, IREG_temp1, IREG_temp0, imm);
                        uop_MEM_STORE_REG(ir, ireg_seg_base(target_seg), IREG_eaaddr, IREG_temp1);
                        uop_MOV(ir, IREG_flags_op1, IREG_temp0);
                        uop_MOV_IMM(ir, IREG_flags_op2, imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ADD32);
                        uop_MOV(ir, IREG_flags_res, IREG_temp1);
                        break;

                        case 0x08: /*OR*/
                        uop_OR_IMM(ir, IREG_temp0, IREG_temp0, imm);
                        uop_MEM_STORE_REG(ir, ireg_seg_base(target_seg), IREG_eaaddr, IREG_temp0);
                        uop_MOV(ir, IREG_flags_res, IREG_temp0);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ZN32);
                        break;

                        case 0x20: /*AND*/
                        uop_AND_IMM(ir, IREG_temp0, IREG_temp0, imm);
                        uop_MEM_STORE_REG(ir, ireg_seg_base(target_seg), IREG_eaaddr, IREG_temp0);
                        uop_MOV(ir, IREG_flags_res, IREG_temp0);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ZN32);
                        break;

                        case 0x28: /*SUB*/
                        uop_SUB_IMM(ir, IREG_temp1, IREG_temp0, imm);
                        uop_MEM_STORE_REG(ir, ireg_seg_base(target_seg), IREG_eaaddr, IREG_temp1);
                        uop_MOV(ir, IREG_flags_op1, IREG_temp0);
                        uop_MOV_IMM(ir, IREG_flags_op2, imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_SUB32);
                        uop_MOV(ir, IREG_flags_res, IREG_temp1);
                        break;

                        case 0x30: /*XOR*/
                        uop_XOR_IMM(ir, IREG_temp0, IREG_temp0, imm);
                        uop_MEM_STORE_REG(ir, ireg_seg_base(target_seg), IREG_eaaddr, IREG_temp0);
                        uop_MOV(ir, IREG_flags_res, IREG_temp0);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ZN32);
                        break;

                        case 0x38: /*CMP*/
                        uop_MOV(ir, IREG_flags_op1, IREG_temp0);
                        uop_MOV_IMM(ir, IREG_flags_op2, imm);
                        uop_SUB_IMM(ir, IREG_flags_res, IREG_temp0, imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_SUB32);
                        break;

                        default:
                        return 0;
                }
        }

        codegen_flags_changed = 1;
        return op_pc + 5;
}

uint32_t rop83_w(codeblock_t *block, ir_data_t *ir, uint8_t opcode, uint32_t fetchdat, uint32_t op_32, uint32_t op_pc)
{
        if ((fetchdat & 0x30) == 0x10) /*ADC/SBB*/
                return 0;

        if ((fetchdat & 0xc0) == 0xc0)
        {
                int dest_reg = fetchdat & 7;
                uint16_t imm = (int16_t)(int8_t)fastreadb(cs + op_pc + 1);

                switch (fetchdat & 0x38)
                {
                        case 0x00: /*ADD*/
                        uop_MOVZX(ir, IREG_flags_op1, IREG_16(dest_reg));
                        uop_ADD_IMM(ir, IREG_16(dest_reg), IREG_16(dest_reg), imm);
                        uop_MOV_IMM(ir, IREG_flags_op2, imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ADD16);
                        uop_MOVZX(ir, IREG_flags_res, IREG_16(dest_reg));
                        break;

                        case 0x08: /*OR*/
                        uop_OR_IMM(ir, IREG_16(dest_reg), IREG_16(dest_reg), imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ZN16);
                        uop_MOVZX(ir, IREG_flags_res, IREG_16(dest_reg));
                        break;

                        case 0x20: /*AND*/
                        uop_AND_IMM(ir, IREG_16(dest_reg), IREG_16(dest_reg), imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ZN16);
                        uop_MOVZX(ir, IREG_flags_res, IREG_16(dest_reg));
                        break;

                        case 0x28: /*SUB*/
                        uop_MOVZX(ir, IREG_flags_op1, IREG_16(dest_reg));
                        uop_SUB_IMM(ir, IREG_16(dest_reg), IREG_16(dest_reg), imm);
                        uop_MOV_IMM(ir, IREG_flags_op2, imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_SUB16);
                        uop_MOVZX(ir, IREG_flags_res, IREG_16(dest_reg));
                        break;

                        case 0x30: /*XOR*/
                        uop_XOR_IMM(ir, IREG_16(dest_reg), IREG_16(dest_reg), imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ZN16);
                        uop_MOVZX(ir, IREG_flags_res, IREG_16(dest_reg));
                        break;

                        case 0x38: /*CMP*/
                        uop_MOVZX(ir, IREG_flags_op1, IREG_16(dest_reg));
                        uop_SUB_IMM(ir, IREG_flags_res_W, IREG_16(dest_reg), imm);
                        uop_MOVZX(ir, IREG_flags_res, IREG_flags_res_W);
                        uop_MOV_IMM(ir, IREG_flags_op2, imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_SUB16);
                        break;

                        default:
                        return 0;
                }
        }
        else
        {
                x86seg *target_seg;
                uint16_t imm;

                uop_MOV_IMM(ir, IREG_oldpc, cpu_state.oldpc);
                target_seg = codegen_generate_ea(ir, op_ea_seg, fetchdat, op_ssegs, &op_pc, op_32);
                if ((fetchdat & 0x38) == 0x38) /*CMP*/
                        codegen_check_seg_read(block, ir, target_seg);
                else
                        codegen_check_seg_write(block, ir, target_seg);
                imm = (int16_t)(int8_t)fastreadb(cs + op_pc + 1);
                uop_MEM_LOAD_REG(ir, IREG_temp0_W, ireg_seg_base(target_seg), IREG_eaaddr);

                switch (fetchdat & 0x38)
                {
                        case 0x00: /*ADD*/
                        uop_ADD_IMM(ir, IREG_temp1_W, IREG_temp0_W, imm);
                        uop_MEM_STORE_REG(ir, ireg_seg_base(target_seg), IREG_eaaddr, IREG_temp1_W);
                        uop_MOVZX(ir, IREG_flags_op1, IREG_temp0_W);
                        uop_MOV_IMM(ir, IREG_flags_op2, imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ADD16);
                        uop_MOVZX(ir, IREG_flags_res, IREG_temp1_W);
                        break;

                        case 0x08: /*OR*/
                        uop_OR_IMM(ir, IREG_temp0_W, IREG_temp0_W, imm);
                        uop_MEM_STORE_REG(ir, ireg_seg_base(target_seg), IREG_eaaddr, IREG_temp0_W);
                        uop_MOVZX(ir, IREG_flags_res, IREG_temp0_W);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ZN16);
                        break;

                        case 0x20: /*AND*/
                        uop_AND_IMM(ir, IREG_temp0_W, IREG_temp0_W, imm);
                        uop_MEM_STORE_REG(ir, ireg_seg_base(target_seg), IREG_eaaddr, IREG_temp0_W);
                        uop_MOVZX(ir, IREG_flags_res, IREG_temp0_W);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ZN16);
                        break;

                        case 0x28: /*SUB*/
                        uop_SUB_IMM(ir, IREG_temp1_W, IREG_temp0_W, imm);
                        uop_MEM_STORE_REG(ir, ireg_seg_base(target_seg), IREG_eaaddr, IREG_temp1_W);
                        uop_MOVZX(ir, IREG_flags_op1, IREG_temp0_W);
                        uop_MOV_IMM(ir, IREG_flags_op2, imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_SUB16);
                        uop_MOVZX(ir, IREG_flags_res, IREG_temp1_W);
                        break;

                        case 0x30: /*XOR*/
                        uop_XOR_IMM(ir, IREG_temp0_W, IREG_temp0_W, imm);
                        uop_MEM_STORE_REG(ir, ireg_seg_base(target_seg), IREG_eaaddr, IREG_temp0_W);
                        uop_MOVZX(ir, IREG_flags_res, IREG_temp0_W);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ZN16);
                        break;

                        case 0x38: /*CMP*/
                        uop_MOVZX(ir, IREG_flags_op1, IREG_temp0_W);
                        uop_MOV_IMM(ir, IREG_flags_op2, imm);
                        uop_SUB_IMM(ir, IREG_flags_res_W, IREG_temp0_W, imm);
                        uop_MOVZX(ir, IREG_flags_res, IREG_flags_res_W);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_SUB16);
                        break;

                        default:
                        return 0;
                }
        }

        codegen_flags_changed = 1;
        return op_pc + 2;
}
uint32_t rop83_l(codeblock_t *block, ir_data_t *ir, uint8_t opcode, uint32_t fetchdat, uint32_t op_32, uint32_t op_pc)
{
        uint32_t imm;

        if ((fetchdat & 0x30) == 0x10) /*ADC/SBB*/
                return 0;
                
        if ((fetchdat & 0xc0) == 0xc0)
        {
                int dest_reg = fetchdat & 7;
                uint32_t imm = (int32_t)(int8_t)fastreadb(cs + op_pc + 1);
                
                switch (fetchdat & 0x38)
                {
                        case 0x00: /*ADD*/
                        uop_MOV(ir, IREG_flags_op1, IREG_32(dest_reg));
                        uop_ADD_IMM(ir, IREG_32(dest_reg), IREG_32(dest_reg), imm);
                        uop_MOV_IMM(ir, IREG_flags_op2, imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ADD32);
                        uop_MOV(ir, IREG_flags_res, IREG_32(dest_reg));
                        break;

                        case 0x08: /*OR*/
                        uop_OR_IMM(ir, IREG_32(dest_reg), IREG_32(dest_reg), imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ZN32);
                        uop_MOV(ir, IREG_flags_res, IREG_32(dest_reg));
                        break;

                        case 0x20: /*AND*/
                        uop_AND_IMM(ir, IREG_32(dest_reg), IREG_32(dest_reg), imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ZN32);
                        uop_MOV(ir, IREG_flags_res, IREG_32(dest_reg));
                        break;

                        case 0x28: /*SUB*/
                        uop_MOV(ir, IREG_flags_op1, IREG_32(dest_reg));
                        uop_SUB_IMM(ir, IREG_32(dest_reg), IREG_32(dest_reg), imm);
                        uop_MOV_IMM(ir, IREG_flags_op2, imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_SUB32);
                        uop_MOV(ir, IREG_flags_res, IREG_32(dest_reg));
                        break;

                        case 0x30: /*XOR*/
                        uop_XOR_IMM(ir, IREG_32(dest_reg), IREG_32(dest_reg), imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ZN32);
                        uop_MOV(ir, IREG_flags_res, IREG_32(dest_reg));
                        break;

                        case 0x38: /*CMP*/
                        uop_MOV(ir, IREG_flags_op1, IREG_32(dest_reg));
                        uop_SUB_IMM(ir, IREG_flags_res, IREG_32(dest_reg), imm);
                        uop_MOV_IMM(ir, IREG_flags_op2, imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_SUB32);
                        break;

                        default:
                        return 0;
                }
        }
        else
        {
                x86seg *target_seg;
                
                uop_MOV_IMM(ir, IREG_oldpc, cpu_state.oldpc);
                target_seg = codegen_generate_ea(ir, op_ea_seg, fetchdat, op_ssegs, &op_pc, op_32);
                if ((fetchdat & 0x38) == 0x38) /*CMP*/
                        codegen_check_seg_read(block, ir, target_seg);
                else
                        codegen_check_seg_write(block, ir, target_seg);
                imm = (int32_t)(int8_t)fastreadb(cs + op_pc + 1);
                uop_MEM_LOAD_REG(ir, IREG_temp0, ireg_seg_base(target_seg), IREG_eaaddr);

                switch (fetchdat & 0x38)
                {
                        case 0x00: /*ADD*/
                        uop_ADD_IMM(ir, IREG_temp1, IREG_temp0, imm);
                        uop_MEM_STORE_REG(ir, ireg_seg_base(target_seg), IREG_eaaddr, IREG_temp1);
                        uop_MOV(ir, IREG_flags_op1, IREG_temp0);
                        uop_MOV_IMM(ir, IREG_flags_op2, imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ADD32);
                        uop_MOV(ir, IREG_flags_res, IREG_temp1);
                        break;

                        case 0x08: /*OR*/
                        uop_OR_IMM(ir, IREG_temp0, IREG_temp0, imm);
                        uop_MEM_STORE_REG(ir, ireg_seg_base(target_seg), IREG_eaaddr, IREG_temp0);
                        uop_MOV(ir, IREG_flags_res, IREG_temp0);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ZN32);
                        break;

                        case 0x20: /*AND*/
                        uop_AND_IMM(ir, IREG_temp0, IREG_temp0, imm);
                        uop_MEM_STORE_REG(ir, ireg_seg_base(target_seg), IREG_eaaddr, IREG_temp0);
                        uop_MOV(ir, IREG_flags_res, IREG_temp0);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ZN32);
                        break;

                        case 0x28: /*SUB*/
                        uop_SUB_IMM(ir, IREG_temp1, IREG_temp0, imm);
                        uop_MEM_STORE_REG(ir, ireg_seg_base(target_seg), IREG_eaaddr, IREG_temp1);
                        uop_MOV(ir, IREG_flags_op1, IREG_temp0);
                        uop_MOV_IMM(ir, IREG_flags_op2, imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_SUB32);
                        uop_MOV(ir, IREG_flags_res, IREG_temp1);
                        break;

                        case 0x30: /*XOR*/
                        uop_XOR_IMM(ir, IREG_temp0, IREG_temp0, imm);
                        uop_MEM_STORE_REG(ir, ireg_seg_base(target_seg), IREG_eaaddr, IREG_temp0);
                        uop_MOV(ir, IREG_flags_res, IREG_temp0);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_ZN32);
                        break;

                        case 0x38: /*CMP*/
                        uop_MOV(ir, IREG_flags_op1, IREG_temp0);
                        uop_MOV_IMM(ir, IREG_flags_op2, imm);
                        uop_SUB_IMM(ir, IREG_flags_res, IREG_temp0, imm);
                        uop_MOV_IMM(ir, IREG_flags_op, FLAGS_SUB32);
                        break;

                        default:
                        return 0;
                }
        }

        codegen_flags_changed = 1;
        return op_pc + 2;
}