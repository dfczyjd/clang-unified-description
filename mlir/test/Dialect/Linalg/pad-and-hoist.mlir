// RUN: mlir-opt %s -test-linalg-codegen-strategy="anchor-op=linalg.matmul pad pack-paddings=1,1,0 hoist-paddings=2,1,0 run-enable-pass=false" -cse -canonicalize -split-input-file | FileCheck %s --check-prefix=HOIST21
// RUN: mlir-opt %s -test-linalg-codegen-strategy="anchor-op=linalg.matmul pad pack-paddings=1,1,0 hoist-paddings=3,2,0 run-enable-pass=false" -cse -canonicalize -split-input-file | FileCheck %s --check-prefix=HOIST32

// HOIST21-DAG: #[[MAP0:[0-9a-z]+]] = affine_map<(d0) -> (5, -d0 + 24)>
// HOIST21-DAG: #[[MAP1:[0-9a-z]+]] = affine_map<(d0) -> (7, -d0 + 25)>
// HOIST21-DAG: #[[MAP2:[0-9a-z]+]] = affine_map<(d0) -> (-d0 + 5)>
// HOIST21-DAG: #[[MAP3:[0-9a-z]+]] = affine_map<(d0) -> (-d0 + 7)>
// HOIST21-DAG: #[[DIV6:[0-9a-z]+]] = affine_map<(d0) -> (d0 ceildiv 6)>
#map0 = affine_map<(d0) -> (5, -d0 + 24)>
#map1 = affine_map<(d0) -> (7, -d0 + 25)>

//      HOIST21:  static_sizes
//      HOIST32:  static_sizes
// HOIST21-SAME:    %[[ARG0:[0-9a-zA-Z]*]]: tensor<24x12xf32>
// HOIST21-SAME:    %[[ARG1:[0-9a-zA-Z]*]]: tensor<12x25xf32>
// HOIST21-SAME:    %[[ARG2:[0-9a-zA-Z]*]]: tensor<24x25xf32>
func @static_sizes(%arg0: tensor<24x12xf32>,
                   %arg1: tensor<12x25xf32>,
                   %arg2: tensor<24x25xf32>) -> tensor<24x25xf32> {
  //  HOIST21-DAG: %[[C0:.*]] = arith.constant 0 : index
  //  HOIST21-DAG: %[[C5:.*]] = arith.constant 5
  //  HOIST21-DAG: %[[C7:.*]] = arith.constant 7
  %c0 = arith.constant 0 : index
  %c12 = arith.constant 12 : index
  %c25 = arith.constant 25 : index
  %c24 = arith.constant 24 : index
  %c6 = arith.constant 6 : index
  %c7 = arith.constant 7 : index
  %c5 = arith.constant 5 : index

  //      HOIST21:  scf.for %[[IV0:[0-9a-zA-Z]*]] =
  %0 = scf.for %arg3 = %c0 to %c24 step %c5 iter_args(%arg4 = %arg2) -> (tensor<24x25xf32>) {

    // Packing the first input operand for all values of IV2 (IV2x5x6).
    //      HOIST21:  = linalg.init_tensor [2, 5, 6]
    //      HOIST21:  %[[PT0:.*]] = scf.for %[[PIV0:[0-9a-z]+]] =
    //        HOIST21:   %[[PIDX0:.*]] = affine.apply #[[DIV6]](%[[PIV0]])
    //        HOIST21:   %[[TS0:.*]] = affine.min #[[MAP0]](%[[IV0]])
    //        HOIST21:   %[[T0:.*]] = tensor.extract_slice %[[ARG0]]
    //   HOIST21-SAME:                                     %[[IV0]], %[[PIV0]]
    //   HOIST21-SAME:                                     %[[TS0]], 6
    //        HOIST21:   %[[V0:.*]] = affine.apply #[[MAP2]](%[[TS0]])
    //        HOIST21:   %[[T1:.*]] = linalg.pad_tensor %[[T0]] nofold {{.*}} high[%[[V0]]
    //        HOIST21:   %[[T2:.*]] = tensor.insert_slice %[[T1:.*]] into %{{.*}}[%[[PIDX0]], 0, 0]
    //        HOIST21:   scf.yield %[[T2:.*]]

    //      HOIST21:  scf.for %[[IV1:[0-9a-zA-Z]*]] =
    %1 = scf.for %arg5 = %c0 to %c25 step %c7 iter_args(%arg6 = %arg4) -> (tensor<24x25xf32>) {

      // Packing the second input operand for all values of IV2 (IV2x6x7).
      //      HOIST21:  = linalg.init_tensor [2, 6, 7]
      //      HOIST21:  %[[PT1:.*]] = scf.for %[[PIV1:[0-9a-z]+]] =
      //        HOIST21:   %[[PIDX1:.*]] = affine.apply #[[DIV6]](%[[PIV1]])
      //        HOIST21:   %[[TS1:.*]] = affine.min #[[MAP1]](%[[IV1]])
      //        HOIST21:   %[[T3:.*]] = tensor.extract_slice %[[ARG1]]
      //   HOIST21-SAME:                                     %[[PIV1]], %[[IV1]]
      //   HOIST21-SAME:                                     6, %[[TS1]]
      //        HOIST21:   %[[V1:.*]] = affine.apply #[[MAP3]](%[[TS1]])
      //        HOIST21:   %[[T4:.*]] = linalg.pad_tensor %[[T3]] nofold {{.*}} high[%[[C0]], %[[V1]]
      //        HOIST21:   %[[T5:.*]] = tensor.insert_slice %[[T4:.*]] into %{{.*}}[%[[PIDX1]], 0, 0]
      //        HOIST21:   scf.yield %[[T5:.*]]

      //      HOIST21:  scf.for %[[IV2:[0-9a-zA-Z]*]] = {{.*}} iter_args(%[[ARG4:.*]] =
      %2 = scf.for %arg7 = %c0 to %c12 step %c6 iter_args(%arg8 = %arg6) -> (tensor<24x25xf32>) {
        %3 = affine.min #map0(%arg3)

        // Index the packed operands.
        //    HOIST21-DAG:   %[[IDX:.*]] = affine.apply #[[DIV6]](%[[IV2]])
        //    HOIST21-DAG:   %[[T6:.*]] = tensor.extract_slice %[[PT0]][%[[IDX]]
        //    HOIST21-DAG:   %[[T7:.*]] = tensor.extract_slice %[[PT1]][%[[IDX]]
        %4 = tensor.extract_slice %arg0[%arg3, %arg7] [%3, 6] [1, 1] : tensor<24x12xf32> to tensor<?x6xf32>
        %5 = affine.min #map1(%arg5)
        %6 = tensor.extract_slice %arg1[%arg7, %arg5] [6, %5] [1, 1] : tensor<12x25xf32> to tensor<6x?xf32>

        // Pad the output operand without setting the nofold attribute.
        //    HOIST21-DAG:   %[[T8:.*]] = tensor.extract_slice %[[ARG4]][%[[IV0]], %[[IV1]]
        //        HOIST21:   %[[T9:.*]] = linalg.pad_tensor %[[T8]] low
        %7 = tensor.extract_slice %arg8[%arg3, %arg5] [%3, %5] [1, 1] : tensor<24x25xf32> to tensor<?x?xf32>

        // Check matmul uses the packed input operands and the padded output operand.
        //        HOIST21:   = linalg.matmul ins(%[[T6]], %[[T7]]{{.*}} outs(%[[T9]]
        %8 = linalg.matmul ins(%4, %6 : tensor<?x6xf32>, tensor<6x?xf32>) outs(%7 : tensor<?x?xf32>) -> tensor<?x?xf32>
        %9 = tensor.insert_slice %8 into %arg8[%arg3, %arg5] [%3, %5] [1, 1] : tensor<?x?xf32> into tensor<24x25xf32>
        scf.yield %9 : tensor<24x25xf32>
      }
      scf.yield %2 : tensor<24x25xf32>
    }
    scf.yield %1 : tensor<24x25xf32>
  }
  return %0 : tensor<24x25xf32>
}

// -----

// HOIST21-DAG: #[[MAP0:[0-9a-z]+]] = affine_map<(d0)[s0] -> (5, -d0 + s0)>
// HOIST21-DAG: #[[MAP1:[0-9a-z]+]] = affine_map<(d0)[s0] -> (6, -d0 + s0)>
// HOIST21-DAG: #[[MAP2:[0-9a-z]+]] = affine_map<(d0)[s0] -> (7, -d0 + s0)>
// HOIST21-DAG: #[[MAP3:[0-9a-z]+]] = affine_map<(d0) -> (-d0 + 5)>
// HOIST21-DAG: #[[MAP4:[0-9a-z]+]] = affine_map<(d0) -> (-d0 + 6)>
// HOIST21-DAG: #[[MAP5:[0-9a-z]+]] = affine_map<(d0) -> (-d0 + 7)>
// HOIST21-DAG: #[[SDIV6:[0-9a-z]+]] = affine_map<()[s0] -> (s0 ceildiv 6)>
// HOIST21-DAG: #[[DDIV6:[0-9a-z]+]] = affine_map<(d0) -> (d0 ceildiv 6)>
#map0 = affine_map<(d0)[s0] -> (5, -d0 + s0)>
#map1 = affine_map<(d0)[s0] -> (6, -d0 + s0)>
#map2 = affine_map<(d0)[s0] -> (7, -d0 + s0)>

//      HOIST21:  dynamic_sizes
//      HOIST32:  dynamic_sizes
// HOIST21-SAME:    %[[ARG0:[0-9a-zA-Z]*]]: tensor<?x?xf32>
// HOIST21-SAME:    %[[ARG1:[0-9a-zA-Z]*]]: tensor<?x?xf32>
// HOIST21-SAME:    %[[ARG2:[0-9a-zA-Z]*]]: tensor<?x?xf32>
func @dynamic_sizes(%arg0: tensor<?x?xf32>,
                    %arg1: tensor<?x?xf32>,
                    %arg2: tensor<?x?xf32>) -> tensor<?x?xf32> {
  //  HOIST21-DAG: %[[C0:.*]] = arith.constant 0 : index
  //  HOIST21-DAG: %[[C1:.*]] = arith.constant 1
  //  HOIST21-DAG: %[[C5:.*]] = arith.constant 5
  //  HOIST21-DAG: %[[C6:.*]] = arith.constant 6
  %c1 = arith.constant 1 : index
  %c0 = arith.constant 0 : index
  %c6 = arith.constant 6 : index
  %c7 = arith.constant 7 : index
  %c5 = arith.constant 5 : index

  //  HOIST21-DAG: %[[D0:.*]] = tensor.dim %[[ARG0]], %[[C0]]
  //  HOIST21-DAG: %[[D1:.*]] = tensor.dim %[[ARG0]], %[[C1]]
  //  HOIST21-DAG: %[[D2:.*]] = tensor.dim %[[ARG1]], %[[C1]]
  %0 = tensor.dim %arg0, %c0 : tensor<?x?xf32>
  %1 = tensor.dim %arg0, %c1 : tensor<?x?xf32>
  %2 = tensor.dim %arg1, %c1 : tensor<?x?xf32>

  //      HOIST21:  scf.for %[[IV0:[0-9a-zA-Z]*]] =
  %3 = scf.for %arg3 = %c0 to %0 step %c5 iter_args(%arg4 = %arg2) -> (tensor<?x?xf32>) {

    // Packing the first input operand for all values of IV2 (IV2x5x6).
    //      HOIST21:  %[[PS0:.*]] = affine.apply #[[SDIV6]]()[%[[D1]]
    //      HOIST21:  = linalg.init_tensor [%[[PS0]], 5, 6]
    //      HOIST21:  %[[PT0:.*]] = scf.for %[[PIV0:[0-9a-z]+]] =
    //        HOIST21:   %[[PIDX0:.*]] = affine.apply #[[DDIV6]](%[[PIV0]])
    //        HOIST21:   %[[TS0:.*]] = affine.min #[[MAP0]](%[[IV0]])[%[[D0]]
    //        HOIST21:   %[[TS1:.*]] = affine.min #[[MAP1]](%[[PIV0]])[%[[D1]]
    //        HOIST21:   %[[T0:.*]] = tensor.extract_slice %[[ARG0]]
    //   HOIST21-SAME:                                     %[[IV0]], %[[PIV0]]
    //   HOIST21-SAME:                                     %[[TS0]], %[[TS1]]
    //        HOIST21:   %[[V0:.*]] = affine.apply #[[MAP3]](%[[TS0]])
    //        HOIST21:   %[[V1:.*]] = affine.apply #[[MAP4]](%[[TS1]])
    //        HOIST21:   %[[T1:.*]] = linalg.pad_tensor %[[T0]] nofold {{.*}} high[%[[V0]], %[[V1]]
    //        HOIST21:   %[[T2:.*]] = tensor.insert_slice %[[T1:.*]] into %{{.*}}[%[[PIDX0]], 0, 0]
    //        HOIST21:   scf.yield %[[T2:.*]]

    //      HOIST21:  scf.for %[[IV1:[0-9a-zA-Z]*]] =
    %4 = scf.for %arg5 = %c0 to %2 step %c7 iter_args(%arg6 = %arg4) -> (tensor<?x?xf32>) {

      // Packing the second input operand for all values of IV2 (IV2x6x7).
      //      HOIST21:  = linalg.init_tensor [%[[PS0]], 6, 7]
      //      HOIST21:  %[[PT1:.*]] = scf.for %[[PIV1:[0-9a-z]+]] =
      //        HOIST21:   %[[PIDX1:.*]] = affine.apply #[[DDIV6]](%[[PIV1]])
      //        HOIST21:   %[[TS2:.*]] = affine.min #[[MAP1]](%[[PIV1]])[%[[D1]]
      //        HOIST21:   %[[TS3:.*]] = affine.min #[[MAP2]](%[[IV1]])[%[[D2]]
      //        HOIST21:   %[[T3:.*]] = tensor.extract_slice %[[ARG1]]
      //   HOIST21-SAME:                                     %[[PIV1]], %[[IV1]]
      //   HOIST21-SAME:                                     %[[TS2]], %[[TS3]]
      //        HOIST21:   %[[V2:.*]] = affine.apply #[[MAP4]](%[[TS2]])
      //        HOIST21:   %[[V3:.*]] = affine.apply #[[MAP5]](%[[TS3]])
      //        HOIST21:   %[[T4:.*]] = linalg.pad_tensor %[[T3]] nofold {{.*}} high[%[[V2]], %[[V3]]
      //        HOIST21:   %[[T5:.*]] = tensor.insert_slice %[[T4:.*]] into %{{.*}}[%[[PIDX1]], 0, 0]
      //        HOIST21:   scf.yield %[[T5:.*]]

      //      HOIST21:  scf.for %[[IV2:[0-9a-zA-Z]*]] = {{.*}} iter_args(%[[ARG4:.*]] =
      %5 = scf.for %arg7 = %c0 to %1 step %c6 iter_args(%arg8 = %arg6) -> (tensor<?x?xf32>) {
        %6 = affine.min #map0(%arg3)[%0]
        %7 = affine.min #map1(%arg7)[%1]

        // Index the packed operands.
        //    HOIST21-DAG:   %[[IDX:.*]] = affine.apply #[[DDIV6]](%[[IV2]])
        //    HOIST21-DAG:   %[[T6:.*]] = tensor.extract_slice %[[PT0]][%[[IDX]]
        //    HOIST21-DAG:   %[[T7:.*]] = tensor.extract_slice %[[PT1]][%[[IDX]]
        %8 = tensor.extract_slice %arg0[%arg3, %arg7] [%6, %7] [1, 1] : tensor<?x?xf32> to tensor<?x?xf32>
        %9 = affine.min #map2(%arg5)[%2]
        %10 = tensor.extract_slice %arg1[%arg7, %arg5] [%7, %9] [1, 1] : tensor<?x?xf32> to tensor<?x?xf32>
        %11 = tensor.extract_slice %arg8[%arg3, %arg5] [%6, %9] [1, 1] : tensor<?x?xf32> to tensor<?x?xf32>

        // Check matmul uses the packed input operands.
        //        HOIST21:   = linalg.matmul ins(%[[T6]], %[[T7]]
        %12 = linalg.matmul ins(%8, %10 : tensor<?x?xf32>, tensor<?x?xf32>) outs(%11 : tensor<?x?xf32>) -> tensor<?x?xf32>
        %13 = tensor.insert_slice %12 into %arg8[%arg3, %arg5] [%6, %9] [1, 1] : tensor<?x?xf32> into tensor<?x?xf32>
        scf.yield %13 : tensor<?x?xf32>
      }
      scf.yield %5 : tensor<?x?xf32>
    }
    scf.yield %4 : tensor<?x?xf32>
  }
  return %3 : tensor<?x?xf32>
}

// -----

// HOIST21-DAG: #[[DIV3:[0-9a-z]+]] = affine_map<(d0) -> (d0 ceildiv 3)>

//      HOIST21:  multiple_operations
//      HOIST32:  multiple_operations
// HOIST21-SAME:    %[[ARG0:[0-9a-zA-Z]*]]: tensor<24x12xf32>
// HOIST21-SAME:    %[[ARG1:[0-9a-zA-Z]*]]: tensor<12x25xf32>
func @multiple_operations(%arg0: tensor<24x12xf32>,
                          %arg1: tensor<12x25xf32>,
                          %arg2: tensor<24x25xf32>) -> tensor<24x25xf32> {
  %c12 = arith.constant 12 : index
  %c3 = arith.constant 3 : index
  %c0 = arith.constant 0 : index
  %c25 = arith.constant 25 : index
  %c24 = arith.constant 24 : index
  %c5 = arith.constant 5 : index
  %c4 = arith.constant 4 : index
  %cst = arith.constant 0.000000e+00 : f32

  //      HOIST21:  scf.for %[[IV0:[0-9a-zA-Z]*]] =
  %0 = scf.for %arg3 = %c0 to %c24 step %c4 iter_args(%arg4 = %arg2) -> (tensor<24x25xf32>) {

    // Packing the first input operand for all values of IV2 (IV2x4x3).
    //      HOIST21:  = linalg.init_tensor [4, 4, 3]
    //      HOIST21:  %[[PT0:.*]] = scf.for %[[PIV0:[0-9a-z]+]] =
    //        HOIST21:   %[[PIDX0:.*]] = affine.apply #[[DIV3]](%[[PIV0]])
    //        HOIST21:   %[[T0:.*]] = tensor.extract_slice %[[ARG0]]
    //        HOIST21:   %[[T1:.*]] = linalg.pad_tensor %[[T0]] nofold
    //        HOIST21:   %[[T2:.*]] = tensor.insert_slice %[[T1:.*]] into %{{.*}}[%[[PIDX0]], 0, 0]
    //        HOIST21:   scf.yield %[[T2:.*]]

    //      HOIST21:  scf.for %[[IV1:[0-9a-zA-Z]*]] =
    %1 = scf.for %arg5 = %c0 to %c25 step %c5 iter_args(%arg6 = %arg4) -> (tensor<24x25xf32>) {
      %2 = tensor.extract_slice %arg6[%arg3, %arg5] [4, 5] [1, 1] : tensor<24x25xf32> to tensor<4x5xf32>

      // Check the fill and pad_tensor ops do not prevent hoisting.
      %3 = linalg.pad_tensor %2 nofold low[%c0, %c0] high[%c0, %c0]  {
      ^bb0(%arg7: index, %arg8: index):  // no predecessors
        linalg.yield %cst : f32
      } : tensor<4x5xf32> to tensor<4x5xf32>
      %4 = linalg.fill(%cst, %3) : f32, tensor<4x5xf32> -> tensor<4x5xf32>

      // Packing the second input operand for all values of IV2 (IV2x3x5).
      //      HOIST21:  = linalg.init_tensor [4, 3, 5]
      //      HOIST21:  %[[PT1:.*]] = scf.for %[[PIV1:[0-9a-z]+]] =
      //        HOIST21:   %[[PIDX1:.*]] = affine.apply #[[DIV3]](%[[PIV1]])
      //        HOIST21:   %[[T3:.*]] = tensor.extract_slice %[[ARG1]]
      //        HOIST21:   %[[T4:.*]] = linalg.pad_tensor %[[T3]] nofold
      //        HOIST21:   %[[T5:.*]] = tensor.insert_slice %[[T4:.*]] into %{{.*}}[%[[PIDX1]], 0, 0]
      //        HOIST21:   scf.yield %[[T5:.*]]

      //      HOIST21:  scf.for %[[IV2:[0-9a-zA-Z]*]] =
      %5 = scf.for %arg7 = %c0 to %c12 step %c3 iter_args(%arg8 = %4) -> (tensor<4x5xf32>) {

        // Index the packed operands.
        //  HOIST21-DAG:   %[[IDX0:.*]] = affine.apply #[[DIV3]](%[[IV2]])
        //  HOIST21-DAG:   %[[T6:.*]] = tensor.extract_slice %[[PT0]][%[[IDX0]]
        //  HOIST21-DAG:   %[[T7:.*]] = tensor.extract_slice %[[PT1]][%[[IDX0]]
        %7 = tensor.extract_slice %arg0[%arg3, %arg7] [4, 3] [1, 1] : tensor<24x12xf32> to tensor<4x3xf32>
        %8 = tensor.extract_slice %arg1[%arg7, %arg5] [3, 5] [1, 1] : tensor<12x25xf32> to tensor<3x5xf32>

        // Check matmul uses the packed input operands.
        //      HOIST21:   = linalg.matmul ins(%[[T6]], %[[T7]]
        %9 = linalg.matmul ins(%7, %8 : tensor<4x3xf32>, tensor<3x5xf32>) outs(%arg8 : tensor<4x5xf32>) -> tensor<4x5xf32>
        scf.yield %9 : tensor<4x5xf32>
      }
      %6 = tensor.insert_slice %5 into %arg6[%arg3, %arg5] [4, 5] [1, 1] : tensor<4x5xf32> into tensor<24x25xf32>
      scf.yield %6 : tensor<24x25xf32>
    }
    scf.yield %1 : tensor<24x25xf32>
  }
  return %0 : tensor<24x25xf32>
}

// -----

// HOIST32-DAG: #[[DIV5:[0-9a-z]+]] = affine_map<(d0) -> (d0 ceildiv 5)>
// HOIST32-DAG: #[[DIV6:[0-9a-z]+]] = affine_map<(d0) -> (d0 ceildiv 6)>
#map0 = affine_map<(d0) -> (15, -d0 + 24)>
#map1 = affine_map<(d0) -> (16, -d0 + 25)>
#map2 = affine_map<(d0, d1) -> (5, -d0 + d1)>
#map3 = affine_map<(d0, d1) -> (d0 + d1)>
#map4 = affine_map<(d0, d1) -> (6, -d0 + d1)>

//      HOIST21:  double_tiling
//      HOIST32:  double_tiling
// HOIST32-SAME:    %[[ARG0:[0-9a-zA-Z]*]]: tensor<24x12xf32>
// HOIST32-SAME:    %[[ARG1:[0-9a-zA-Z]*]]: tensor<12x25xf32>
func @double_tiling(%arg0: tensor<24x12xf32>,
                    %arg1: tensor<12x25xf32>,
                    %arg2: tensor<24x25xf32>) -> tensor<24x25xf32> {
  %c15 = arith.constant 15 : index
  %c16 = arith.constant 16 : index
  %c24 = arith.constant 24 : index
  %c25 = arith.constant 25 : index
  %c0 = arith.constant 0 : index
  %c5 = arith.constant 5 : index
  %c6 = arith.constant 6 : index

  //    HOIST32:  scf.for %[[IV0:[0-9a-zA-Z]*]] =
  %0 = scf.for %arg3 = %c0 to %c24 step %c15 iter_args(%arg4 = %arg2) -> (tensor<24x25xf32>) {

    // Packing the first input operand.
    //    HOIST32:  = linalg.init_tensor [3, 5, 12]
    //    HOIST32:  %[[PT0:.*]] = scf.for %[[PIV0:[0-9a-z]+]] =
    //      HOIST32:   %[[PIDX0:.*]] = affine.apply #[[DIV5]](%[[PIV0]])
    //      HOIST32:   %[[T0:.*]] = tensor.extract_slice %[[ARG0]]
    //      HOIST32:   %[[T1:.*]] = linalg.pad_tensor %[[T0]] nofold
    //      HOIST32:   %[[T2:.*]] = tensor.insert_slice %[[T1:.*]] into %{{.*}}[%[[PIDX0]], 0, 0]
    //      HOIST32:   scf.yield %[[T2:.*]]

    //    HOIST32:  scf.for %[[IV1:[0-9a-zA-Z]*]] =
    %1 = scf.for %arg5 = %c0 to %c25 step %c16 iter_args(%arg6 = %arg4) -> (tensor<24x25xf32>) {
      %2 = affine.min #map0(%arg3)
      %3 = affine.min #map1(%arg5)
      %4 = tensor.extract_slice %arg6[%arg3, %arg5] [%2, %3] [1, 1] : tensor<24x25xf32> to tensor<?x?xf32>

      // Packing the second input operand.
      //    HOIST32:  = linalg.init_tensor [3, 12, 6]
      //    HOIST32:  %[[PT1:.*]] = scf.for %[[PIV1:[0-9a-z]+]] =
      //      HOIST32:   %[[PIDX1:.*]] = affine.apply #[[DIV6]](%[[PIV1]])
      //      HOIST32:   %[[T3:.*]] = tensor.extract_slice %[[ARG1]]
      //      HOIST32:   %[[T4:.*]] = linalg.pad_tensor %[[T3]] nofold
      //      HOIST32:   %[[T5:.*]] = tensor.insert_slice %[[T4:.*]] into %{{.*}}[%[[PIDX1]], 0, 0]
      //      HOIST32:   scf.yield %[[T5:.*]]

      //    HOIST32:  scf.for %[[IV2:[0-9a-zA-Z]*]] =
      %5 = scf.for %arg7 = %c0 to %2 step %c5 iter_args(%arg8 = %4) -> (tensor<?x?xf32>) {

        //    HOIST32:  scf.for %[[IV3:[0-9a-zA-Z]*]] =
        %7 = scf.for %arg9 = %c0 to %3 step %c6 iter_args(%arg10 = %arg8) -> (tensor<?x?xf32>) {
          %8 = affine.min #map2(%arg7, %2)
          %9 = affine.apply #map3(%arg7, %arg3)

          // Index the packed operands.
          //    HOIST32-DAG:   %[[IDX0:.*]] = affine.apply #[[DIV5]](%[[IV2]])
          //    HOIST32-DAG:   %[[T6:.*]] = tensor.extract_slice %[[PT0]][%[[IDX0]]
          //    HOIST32-DAG:   %[[IDX1:.*]] = affine.apply #[[DIV6]](%[[IV3]])
          //    HOIST32-DAG:   %[[T7:.*]] = tensor.extract_slice %[[PT1]][%[[IDX1]]
          %10 = tensor.extract_slice %arg0[%9, 0] [%8, 12] [1, 1] : tensor<24x12xf32> to tensor<?x12xf32>
          %11 = affine.min #map4(%arg9, %3)
          %12 = affine.apply #map3(%arg9, %arg5)
          %13 = tensor.extract_slice %arg1[0, %12] [12, %11] [1, 1] : tensor<12x25xf32> to tensor<12x?xf32>
          %14 = affine.min #map2(%arg7, %2)
          %15 = affine.min #map4(%arg9, %3)
          %16 = tensor.extract_slice %arg10[%arg7, %arg9] [%14, %15] [1, 1] : tensor<?x?xf32> to tensor<?x?xf32>

          // Check matmul uses the packed input operands.
          //    HOIST32:   = linalg.matmul ins(%[[T6]], %[[T7]]
          %17 = linalg.matmul ins(%10, %13 : tensor<?x12xf32>, tensor<12x?xf32>) outs(%16 : tensor<?x?xf32>) -> tensor<?x?xf32>
          %18 = tensor.insert_slice %17 into %arg10[%arg7, %arg9] [%14, %15] [1, 1] : tensor<?x?xf32> into tensor<?x?xf32>
          scf.yield %18 : tensor<?x?xf32>
        }
        scf.yield %7 : tensor<?x?xf32>
      }
      %6 = tensor.insert_slice %5 into %arg6[%arg3, %arg5] [%2, %3] [1, 1] : tensor<?x?xf32> into tensor<24x25xf32>
      scf.yield %6 : tensor<24x25xf32>
    }
    scf.yield %1 : tensor<24x25xf32>
  }
  return %0 : tensor<24x25xf32>
}

// -----

#map0 = affine_map<(d0) -> (5, -d0 + 24)>
#map1 = affine_map<(d0) -> (7, -d0 + 25)>
#map2 = affine_map<(d0) -> (-d0 + 5)>
#map3 = affine_map<(d0) -> (-d0 + 7)>

// Note that the input ir of the test already contains the padding. As a result,
// the padding pattern skips the padding and performs hoisting only, which the
// unexpected_loop test verifies by hoisting one of the input operands.

//      HOIST21:  non_constant_padding
//      HOIST32:  non_constant_padding
// HOIST21-SAME:    %[[ARG1:[0-9a-zA-Z]*]]: tensor<12x25xf32>
func @non_constant_padding(%arg0: tensor<24x12xf32>,
                           %arg1: tensor<12x25xf32>,
                           %arg2: tensor<24x25xf32>) -> tensor<24x25xf32> {
  %c0 = arith.constant 0 : index
  %c12 = arith.constant 12 : index
  %c25 = arith.constant 25 : index
  %c24 = arith.constant 24 : index
  %c6 = arith.constant 6 : index
  %c7 = arith.constant 7 : index
  %c5 = arith.constant 5 : index
  %cst = arith.constant 0.000000e+00 : f32

  //      HOIST21:  scf.for %[[IV0:[0-9a-zA-Z]*]] =
  %0 = scf.for %arg3 = %c0 to %c24 step %c5 iter_args(%arg4 = %arg2) -> (tensor<24x25xf32>) {

    // HOIST21-NEXT:  scf.for %[[IV1:[0-9a-zA-Z]*]] =
    %1 = scf.for %arg5 = %c0 to %c25 step %c7 iter_args(%arg6 = %arg4) -> (tensor<24x25xf32>) {

      // HOIST21-NEXT:  scf.for %[[IV2:[0-9a-zA-Z]*]] =
      %2 = scf.for %arg7 = %c0 to %c12 step %c6 iter_args(%arg8 = %arg6) -> (tensor<24x25xf32>) {
        %3 = affine.min #map0(%arg3)
        %4 = tensor.extract_slice %arg0[%arg3, %arg7] [%3, 6] [1, 1] : tensor<24x12xf32> to tensor<?x6xf32>
        %5 = affine.min #map1(%arg5)
        %6 = tensor.extract_slice %arg1[%arg7, %arg5] [6, %5] [1, 1] : tensor<12x25xf32> to tensor<6x?xf32>
        %7 = tensor.extract_slice %arg8[%arg3, %arg5] [%3, %5] [1, 1] : tensor<24x25xf32> to tensor<?x?xf32>
        %8 = affine.apply #map2(%3)

        // Check the padding with a non constant padding value is not hoisted.
        //      HOIST21:  %[[T0:.*]] = linalg.pad_tensor
        //      HOIST21:    %[[V0:.*]] = arith.index_cast
        //      HOIST21:    %[[V1:.*]] = arith.sitofp %[[V0]]
        //      HOIST21:     linalg.yield %[[V1]]
        %9 = linalg.pad_tensor %4 nofold low[%c0, %c0] high[%8, %c0]  {
        ^bb0(%arg9: index, %arg10: index):  // no predecessors
          %17 = arith.index_cast %arg7 : index to i32
          %18 = arith.sitofp %17 : i32 to f32
          linalg.yield %18 : f32
        } : tensor<?x6xf32> to tensor<5x6xf32>
        %10 = affine.apply #map3(%5)

        // Check the padding with a non constant op padding is not hoisted.
        //      HOIST21:  %[[V2:.*]] = tensor.extract %[[ARG1]][%[[IV2]], %[[IV1]]
        //      HOIST21:  %[[T1:.*]] = linalg.pad_tensor
        //      HOIST21:    linalg.yield %[[V2]]
        %11 = tensor.extract %arg1[%arg7, %arg5] : tensor<12x25xf32>
        %12 = linalg.pad_tensor %6 nofold low[%c0, %c0] high[%c0, %10]  {
        ^bb0(%arg9: index, %arg10: index):  // no predecessors
          linalg.yield %11 : f32
        } : tensor<6x?xf32> to tensor<6x7xf32>
        %13 = linalg.pad_tensor %7 low[%c0, %c0] high[%8, %10]  {
        ^bb0(%arg9: index, %arg10: index):  // no predecessors
          linalg.yield %cst : f32
        } : tensor<?x?xf32> to tensor<5x7xf32>

        //      HOIST21:  = linalg.matmul ins(%[[T0]], %[[T1]]
        %14 = linalg.matmul ins(%9, %12 : tensor<5x6xf32>, tensor<6x7xf32>) outs(%13 : tensor<5x7xf32>) -> tensor<5x7xf32>
        %15 = tensor.extract_slice %14[0, 0] [%3, %5] [1, 1] : tensor<5x7xf32> to tensor<?x?xf32>
        %16 = tensor.insert_slice %15 into %arg8[%arg3, %arg5] [%3, %5] [1, 1] : tensor<?x?xf32> into tensor<24x25xf32>
        scf.yield %16 : tensor<24x25xf32>
      }
      scf.yield %2 : tensor<24x25xf32>
    }
    scf.yield %1 : tensor<24x25xf32>
  }
  return %0 : tensor<24x25xf32>
}

// -----

#map0 = affine_map<(d0) -> (5, -d0 + 24)>
#map1 = affine_map<(d0) -> (7, -d0 + 25)>
#map2 = affine_map<(d0) -> (-d0 + 5)>
#map3 = affine_map<(d0) -> (-d0 + 7)>

// Note that the input ir of the test already contains the padding. As a result,
// the padding pattern skips the padding and performs hoisting only, which the
// unexpected_loop test verifies by hoisting one of the input operands.

//      HOIST21:  unexpected_operation
//      HOIST32:  unexpected_operation
// HOIST21-SAME:    %[[ARG3:[0-9a-zA-Z]*]]: memref<?xindex>
// HOIST21-SAME:    %[[ARG4:[0-9a-zA-Z]*]]: i32
func @unexpected_operation(%arg0: tensor<24x12xf32>,
                           %arg1: tensor<12x25xf32>,
                           %arg2: tensor<24x25xf32>,
                           %arg3: memref<?xindex>,
                           %arg4: i32) -> tensor<24x25xf32> {
  %cst = arith.constant 0.000000e+00 : f32
  %c5 = arith.constant 5 : index
  %c7 = arith.constant 7 : index
  %c6 = arith.constant 6 : index
  %c24 = arith.constant 24 : index
  %c25 = arith.constant 25 : index
  %c12 = arith.constant 12 : index
  %c0 = arith.constant 0 : index

  //      HOIST21:  scf.for %[[IV0:[0-9a-zA-Z]*]] =
  %0 = scf.for %arg5 = %c0 to %c24 step %c5 iter_args(%arg6 = %arg2) -> (tensor<24x25xf32>) {

    // HOIST21-NEXT:  scf.for %[[IV1:[0-9a-zA-Z]*]] =
    %1 = scf.for %arg7 = %c0 to %c25 step %c7 iter_args(%arg8 = %arg6) -> (tensor<24x25xf32>) {

      // HOIST21-NEXT:  scf.for %[[IV2:[0-9a-zA-Z]*]] =
      %2 = scf.for %arg9 = %c0 to %c12 step %c6 iter_args(%arg10 = %arg8) -> (tensor<24x25xf32>) {
        %3 = affine.min #map0(%arg5)
        %4 = tensor.extract_slice %arg0[%arg5, %arg9] [%3, 6] [1, 1] : tensor<24x12xf32> to tensor<?x6xf32>
        %5 = affine.min #map1(%arg7)
        %6 = tensor.extract_slice %arg1[%arg9, %arg7] [6, %5] [1, 1] : tensor<12x25xf32> to tensor<6x?xf32>
        %7 = tensor.extract_slice %arg10[%arg5, %arg7] [%3, %5] [1, 1] : tensor<24x25xf32> to tensor<?x?xf32>
        %8 = affine.apply #map2(%3)

        // Check cannot hoist due to unexpected operation with memory effect.
        //      HOIST21: %[[IDX0:.*]] = memref.load %[[ARG3]]
        //      HOIST21: %[[T0:.*]] = linalg.pad_tensor {{.*}}, %[[IDX0]]
        %9 = memref.load %arg3[%c0] : memref<?xindex>
        %10 = linalg.pad_tensor %4 nofold low[%c0, %c0] high[%8, %9]  {
        ^bb0(%arg11: index, %arg12: index):  // no predecessors
          linalg.yield %cst : f32
        } : tensor<?x6xf32> to tensor<5x6xf32>
        %11 = affine.apply #map3(%5)

        // Check cannot hoist due to unexpected operation with non index operand.
        //      HOIST21: %[[IDX1:.*]] = arith.index_cast %[[ARG4]]
        //      HOIST21: %[[T1:.*]] = linalg.pad_tensor {{.*}}[%[[IDX1]]
        %12 = arith.index_cast %arg4 : i32 to index
        %13 = linalg.pad_tensor %6 nofold low[%c0, %c0] high[%12, %11]  {
        ^bb0(%arg11: index, %arg12: index):  // no predecessors
          linalg.yield %cst : f32
        } : tensor<6x?xf32> to tensor<6x7xf32>
        %14 = linalg.pad_tensor %7 low[%c0, %c0] high[%8, %11]  {
        ^bb0(%arg11: index, %arg12: index):  // no predecessors
          linalg.yield %cst : f32
        } : tensor<?x?xf32> to tensor<5x7xf32>

        //      HOIST21:  = linalg.matmul ins(%[[T0]], %[[T1]]
        %15 = linalg.matmul ins(%10, %13 : tensor<5x6xf32>, tensor<6x7xf32>) outs(%14 : tensor<5x7xf32>) -> tensor<5x7xf32>
        %16 = tensor.extract_slice %15[0, 0] [%3, %5] [1, 1] : tensor<5x7xf32> to tensor<?x?xf32>
        %17 = tensor.insert_slice %16 into %arg10[%arg5, %arg7] [%3, %5] [1, 1] : tensor<?x?xf32> into tensor<24x25xf32>
        scf.yield %17 : tensor<24x25xf32>
      }
      scf.yield %2 : tensor<24x25xf32>
    }
    scf.yield %1 : tensor<24x25xf32>
  }
  return %0 : tensor<24x25xf32>
}

// -----

#map0 = affine_map<(d0) -> (5, -d0 + 24)>
#map1 = affine_map<(d0) -> (7, -d0 + 25)>
#map2 = affine_map<(d0) -> (-d0 + 5)>
#map3 = affine_map<(d0) -> (-d0 + 7)>

// Note that the input ir of the test already contains the padding. As a result,
// the padding pattern skips the padding and performs hoisting only, which the
// test verifies by hoisting one of the input operands.

//      HOIST21:  unexpected_loop
//      HOIST32:  unexpected_loop
// HOIST21-SAME:    %[[ARG3:[0-9a-zA-Z]*]]: index
func @unexpected_loop(%arg0: tensor<24x12xf32>,
                      %arg1: tensor<12x25xf32>,
                      %arg2: tensor<24x25xf32>,
                      %arg3: index) -> tensor<24x25xf32> {
  %c0 = arith.constant 0 : index
  %c12 = arith.constant 12 : index
  %c25 = arith.constant 25 : index
  %c24 = arith.constant 24 : index
  %c6 = arith.constant 6 : index
  %c7 = arith.constant 7 : index
  %c5 = arith.constant 5 : index
  %cst = arith.constant 0.000000e+00 : f32

  //      HOIST21:  scf.for %[[IV0:[0-9a-zA-Z]*]] =
  %0 = scf.for %arg4 = %c0 to %c24 step %c5 iter_args(%arg5 = %arg2) -> (tensor<24x25xf32>) {

    // HOIST21-NEXT:  scf.for %[[IV1:[0-9a-zA-Z]*]] =
    %1 = scf.for %arg6 = %c0 to %c25 step %c7 iter_args(%arg7 = %arg5) -> (tensor<24x25xf32>) {

      // Check the padding of the first input operand is hoisted.
      //      HOIST21:  = linalg.pad_tensor

      //      HOIST21:  scf.for %[[IV2:[0-9a-zA-Z]*]] =
      %2 = scf.for %arg8 = %c0 to %c12 step %c6 iter_args(%arg9 = %arg7) -> (tensor<24x25xf32>) {
        %3 = affine.min #map0(%arg4)
        %4 = tensor.extract_slice %arg0[%arg4, %arg8] [%3, 6] [1, 1] : tensor<24x12xf32> to tensor<?x6xf32>
        %5 = affine.min #map1(%arg6)
        %6 = tensor.extract_slice %arg1[%arg8, %arg6] [6, %5] [1, 1] : tensor<12x25xf32> to tensor<6x?xf32>
        %7 = tensor.extract_slice %arg9[%arg4, %arg6] [%3, %5] [1, 1] : tensor<24x25xf32> to tensor<?x?xf32>
        %8 = affine.apply #map2(%3)

        // Check cannot hoist due to unexpected operation that has a region.
        //      HOIST21: %[[IDX0:.*]] = scf.for {{.*}} step %[[ARG3]]
        //      HOIST21: %[[T0:.*]] = linalg.pad_tensor {{.*}}, %[[IDX0]]
        %9 = scf.for %arg10 = %c0 to %c24 step %arg3 iter_args(%arg11 = %c0) -> (index) {
          %17 = arith.addi %arg3, %arg11 : index
          scf.yield %17 : index
        }
        %10 = linalg.pad_tensor %4 nofold low[%c0, %c0] high[%8, %9]  {
        ^bb0(%arg10: index, %arg11: index):  // no predecessors
          linalg.yield %cst : f32
        } : tensor<?x6xf32> to tensor<5x6xf32>
        %11 = affine.apply #map3(%5)
        %12 = linalg.pad_tensor %6 nofold low[%c0, %c0] high[%c0, %11]  {
        ^bb0(%arg10: index, %arg11: index):  // no predecessors
          linalg.yield %cst : f32
        } : tensor<6x?xf32> to tensor<6x7xf32>
        %13 = linalg.pad_tensor %7 low[%c0, %c0] high[%8, %11]  {
        ^bb0(%arg10: index, %arg11: index):  // no predecessors
          linalg.yield %cst : f32
        } : tensor<?x?xf32> to tensor<5x7xf32>

        //      HOIST21:  = linalg.matmul ins(%[[T0]]
        %14 = linalg.matmul ins(%10, %12 : tensor<5x6xf32>, tensor<6x7xf32>) outs(%13 : tensor<5x7xf32>) -> tensor<5x7xf32>
        %15 = tensor.extract_slice %14[0, 0] [%3, %5] [1, 1] : tensor<5x7xf32> to tensor<?x?xf32>
        %16 = tensor.insert_slice %15 into %arg9[%arg4, %arg6] [%3, %5] [1, 1] : tensor<?x?xf32> into tensor<24x25xf32>
        scf.yield %16 : tensor<24x25xf32>
      }
      scf.yield %2 : tensor<24x25xf32>
    }
    scf.yield %1 : tensor<24x25xf32>
  }
  return %0 : tensor<24x25xf32>
}

