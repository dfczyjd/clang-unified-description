//===- X86VectorDialect.cpp - MLIR X86Vector ops implementation -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the X86Vector dialect and its operations.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/X86Vector/X86VectorDialect.h"
#include "mlir/Dialect/LLVMIR/LLVMTypes.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/IR/TypeUtilities.h"
#include "mlir/Interfaces/InferTypeOpInterface.h"

using namespace mlir;

#include "mlir/Dialect/X86Vector/X86VectorDialect.cpp.inc"

void x86vector::X86VectorDialect::initialize() {
  addOperations<
#define GET_OP_LIST
#include "mlir/Dialect/X86Vector/X86Vector.cpp.inc"
      >();
}

static LogicalResult verify(x86vector::MaskCompressOp op) {
  if (op.src() && op.constant_src())
    return emitError(op.getLoc(), "cannot use both src and constant_src");

  if (op.src() && (op.src().getType() != op.dst().getType()))
    return emitError(op.getLoc(),
                     "failed to verify that src and dst have same type");

  if (op.constant_src() && (op.constant_src()->getType() != op.dst().getType()))
    return emitError(
        op.getLoc(),
        "failed to verify that constant_src and dst have same type");

  return success();
}

#define GET_OP_CLASSES
#include "mlir/Dialect/X86Vector/X86Vector.cpp.inc"
