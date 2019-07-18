//
//  Model
//  Blimpy
//
//  Created by Joël Gähwiler on 18.07.19.
//  Copyright © 2019 Joël Gähwiler. All rights reserved.
//

import Foundation

let FORCE_MAX = 721.24891681

let FORCE_MAT = [[ 0.35355339, -0.35355339],
             [ 0.35355339,  0.35355339],
             [ 0.35355339,  0.35355339],
             [ 0.35355339, -0.35355339]]

func calcForce(x: Double, z: Double) -> Array<Double> {
    // get forces
    let fx = x * FORCE_MAX
    let fz = z * FORCE_MAX
    
    // get motor speeds
    let m1 = FORCE_MAT[0][0] * fx + FORCE_MAT[0][1] * fz
    let m2 = FORCE_MAT[1][0] * fx + FORCE_MAT[1][1] * fz
    let m3 = FORCE_MAT[2][0] * fx + FORCE_MAT[2][1] * fz
    let m4 = FORCE_MAT[3][0] * fx + FORCE_MAT[3][1] * fz
    
    return [m1, m2, m3, m4]
}
