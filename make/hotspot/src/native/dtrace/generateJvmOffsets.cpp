/*
 * Copyright (c) 2003, 2019, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

/*
 * This is to provide sanity check in jhelper.d which compares SCCS
 * versions of generateJvmOffsets.cpp used to create and extract
 * contents of __JvmOffsets[] table.
 * The __JvmOffsets[] table is located in generated JvmOffsets.cpp.
 *
 * GENOFFS_SCCS_VER 34
 */

#include <stdio.h>
#include <strings.h>

/* A workaround for private and protected fields */
#define private   public
#define protected public

#include <proc_service.h>
#include "gc/shared/collectedHeap.hpp"
#include "memory/heap.hpp"
#include "oops/compressedOops.hpp"
#include "runtime/vmStructs.hpp"

typedef enum GEN_variant {
        GEN_OFFSET = 0,
        GEN_INDEX  = 1,
        GEN_TABLE  = 2
} GEN_variant;

#ifdef COMPILER1
#ifdef ASSERT

/*
 * To avoid the most part of potential link errors
 * we link this program with -z nodefs .
 *
 * But for 'debug1' and 'fastdebug1' we still have to provide
 * a particular workaround for the following symbols below.
 * It will be good to find out a generic way in the future.
 */

#pragma weak tty

#if defined(i386) || defined(__i386) || defined(__amd64)
#pragma weak noreg
#endif /* i386 */

LIR_Opr LIR_OprFact::illegalOpr = (LIR_Opr) 0;

address StubRoutines::_call_stub_return_address = NULL;

StubQueue* AbstractInterpreter::_code = NULL;

#endif /* ASSERT */
#endif /* COMPILER1 */

#define GEN_OFFS_NAME(Type,Name,OutputType)             \
  switch(gen_variant) {                                 \
  case GEN_OFFSET:                                      \
    printf("#define OFFSET_%-33s %ld\n",                \
            #OutputType #Name, offset_of(Type, Name));  \
    break;                                              \
  case GEN_INDEX:                                       \
    printf("#define IDX_OFFSET_%-33s %d\n",             \
            #OutputType #Name, index++);                \
    break;                                              \
  case GEN_TABLE:                                       \
    printf("\tOFFSET_%s,\n", #OutputType #Name);        \
    break;                                              \
  }

#define GEN_OFFS(Type,Name)                             \
  GEN_OFFS_NAME(Type,Name,Type)

#define GEN_SIZE(Type)                                  \
  switch(gen_variant) {                                 \
  case GEN_OFFSET:                                      \
    printf("#define SIZE_%-35s %ld\n",                  \
            #Type, sizeof(Type));                       \
    break;                                              \
  case GEN_INDEX:                                       \
    printf("#define IDX_SIZE_%-35s %d\n",               \
            #Type, index++);                            \
    break;                                              \
  case GEN_TABLE:                                       \
    printf("\tSIZE_%s,\n", #Type);                      \
    break;                                              \
  }

#define GEN_VALUE(String,Value)                         \
  switch(gen_variant) {                                 \
  case GEN_OFFSET:                                      \
    printf("#define %-40s %d\n", #String, Value);       \
    break;                                              \
  case GEN_INDEX:                                       \
    printf("#define IDX_%-40s %d\n", #String, index++); \
    break;                                              \
  case GEN_TABLE:                                       \
    printf("\t" #String ",\n");                         \
    break;                                              \
  }

void gen_prologue(GEN_variant gen_variant) {
    const char *suffix = "Undefined-Suffix";

    switch(gen_variant) {
      case GEN_OFFSET: suffix = ".h";        break;
      case GEN_INDEX:  suffix = "Index.h";   break;
      case GEN_TABLE:  suffix = ".cpp";      break;
    }

    printf("/*\n");
    printf(" * JvmOffsets%s !!!DO NOT EDIT!!! \n", suffix);
    printf(" * The generateJvmOffsets program generates this file!\n");
    printf(" */\n\n");
    switch(gen_variant) {

      case GEN_OFFSET:
      case GEN_INDEX:
        break;

      case GEN_TABLE:
        printf("#include \"JvmOffsets.h\"\n");
        printf("\n");
        printf("int __JvmOffsets[] = {\n");
        break;
    }
}

void gen_epilogue(GEN_variant gen_variant) {
    if (gen_variant != GEN_TABLE) {
        return;
    }
    printf("};\n\n");
    return;
}

int generateJvmOffsets(GEN_variant gen_variant) {
  int index = 0;        /* It is used to generate JvmOffsetsIndex.h */
  int pointer_size = sizeof(void *);
  int data_model = (pointer_size == 4) ? PR_MODEL_ILP32 : PR_MODEL_LP64;

  gen_prologue(gen_variant);

  GEN_VALUE(DATA_MODEL, data_model);
  GEN_VALUE(POINTER_SIZE, pointer_size);
#if defined(TIERED)
  GEN_VALUE(COMPILER, 3);
#elif COMPILER1
  GEN_VALUE(COMPILER, 1);
#elif COMPILER2
  GEN_VALUE(COMPILER, 2);
#else
  GEN_VALUE(COMPILER, 0);
#endif // COMPILER1 && COMPILER2
  printf("\n");

  GEN_OFFS(CollectedHeap, _reserved);
  GEN_OFFS(MemRegion, _start);
  GEN_OFFS(MemRegion, _word_size);
  GEN_SIZE(HeapWord);
  printf("\n");

  GEN_OFFS(VMStructEntry, typeName);
  GEN_OFFS(VMStructEntry, fieldName);
  GEN_OFFS(VMStructEntry, address);
  GEN_SIZE(VMStructEntry);
  printf("\n");

  GEN_VALUE(MAX_METHOD_CODE_SIZE, max_method_code_size);
#if defined(i386) || defined(__i386) || defined(__amd64)
  GEN_VALUE(OFFSET_interpreter_frame_sender_sp, -1 * pointer_size);
  GEN_VALUE(OFFSET_interpreter_frame_method, -3 * pointer_size);
  GEN_VALUE(OFFSET_interpreter_frame_bcp_offset, -7 * pointer_size);
#endif

  GEN_OFFS(Klass, _name);
  GEN_OFFS(ConstantPool, _pool_holder);
  printf("\n");

  GEN_VALUE(OFFSET_HeapBlockHeader_used, (int) offset_of(HeapBlock::Header, _used));
  GEN_OFFS(oopDesc, _metadata);
  printf("\n");

  GEN_VALUE(AccessFlags_NATIVE, JVM_ACC_NATIVE);
  GEN_VALUE(ConstMethod_has_linenumber_table, ConstMethod::_has_linenumber_table);
  GEN_OFFS(AccessFlags, _flags);
  GEN_OFFS(Symbol, _length);
  GEN_OFFS(Symbol, _body);
  printf("\n");

  GEN_OFFS(Method, _constMethod);
  GEN_OFFS(Method, _access_flags);
  printf("\n");

  GEN_OFFS(ConstMethod, _constants);
  GEN_OFFS(ConstMethod, _flags);
  GEN_OFFS(ConstMethod, _code_size);
  GEN_OFFS(ConstMethod, _name_index);
  GEN_OFFS(ConstMethod, _signature_index);
  printf("\n");

  GEN_OFFS(CodeHeap, _memory);
  GEN_OFFS(CodeHeap, _segmap);
  GEN_OFFS(CodeHeap, _log2_segment_size);
  printf("\n");

  GEN_OFFS(VirtualSpace, _low_boundary);
  GEN_OFFS(VirtualSpace, _high_boundary);
  GEN_OFFS(VirtualSpace, _low);
  GEN_OFFS(VirtualSpace, _high);
  printf("\n");

  /* We need to use different names here because of the template parameter */
  GEN_OFFS_NAME(GrowableArray<CodeHeap*>, _data, GrowableArray_CodeHeap);
  GEN_OFFS_NAME(GrowableArray<CodeHeap*>, _len, GrowableArray_CodeHeap);
  printf("\n");

  GEN_OFFS(CodeBlob, _name);
  GEN_OFFS(CodeBlob, _header_size);
  GEN_OFFS(CodeBlob, _content_begin);
  GEN_OFFS(CodeBlob, _code_begin);
  GEN_OFFS(CodeBlob, _code_end);
  GEN_OFFS(CodeBlob, _data_offset);
  GEN_OFFS(CodeBlob, _frame_size);
  printf("\n");

  GEN_OFFS(nmethod, _method);
  GEN_OFFS(nmethod, _dependencies_offset);
  GEN_OFFS(nmethod, _metadata_offset);
  GEN_OFFS(nmethod, _scopes_data_begin);
  GEN_OFFS(nmethod, _scopes_pcs_offset);
  GEN_OFFS(nmethod, _handler_table_offset);
  GEN_OFFS(nmethod, _deopt_handler_begin);
  GEN_OFFS(nmethod, _orig_pc_offset);

  GEN_OFFS(PcDesc, _pc_offset);
  GEN_OFFS(PcDesc, _scope_decode_offset);

  printf("\n");

  GEN_OFFS(NarrowPtrStruct, _base);
  GEN_OFFS(NarrowPtrStruct, _shift);
  printf("\n");

  GEN_VALUE(SIZE_HeapBlockHeader, (int) sizeof(HeapBlock::Header));
  GEN_SIZE(oopDesc);
  GEN_SIZE(ConstantPool);
  printf("\n");

  GEN_SIZE(PcDesc);
  GEN_SIZE(Method);
  GEN_SIZE(ConstMethod);
  GEN_SIZE(nmethod);
  GEN_SIZE(CodeBlob);
  GEN_SIZE(BufferBlob);
  GEN_SIZE(SingletonBlob);
  GEN_SIZE(RuntimeStub);
  GEN_SIZE(SafepointBlob);

  gen_epilogue(gen_variant);
  printf("\n");

  fflush(stdout);
  return 0;
}

const char *HELP =
    "HELP: generateJvmOffsets {-header | -index | -table} \n";

int main(int argc, const char *argv[]) {
    GEN_variant gen_var;

    if (argc != 2) {
        printf("%s", HELP);
        return 1;
    }

    if (0 == strcmp(argv[1], "-header")) {
        gen_var = GEN_OFFSET;
    }
    else if (0 == strcmp(argv[1], "-index")) {
        gen_var = GEN_INDEX;
    }
    else if (0 == strcmp(argv[1], "-table")) {
        gen_var = GEN_TABLE;
    }
    else {
        printf("%s", HELP);
        return 1;
    }
    return generateJvmOffsets(gen_var);
}
