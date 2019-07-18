//
//  Model
//  Blimpy
//
//  Created by Joël Gähwiler on 18.07.19.
//  Copyright © 2019 Joël Gähwiler. All rights reserved.
//

import Foundation

let FORCE_MAX = 721.24891681
let TORQUE_MAX = 360.6244584051392

let MATRICE = [[ 0.35355339, -0.35355339,  0.70710678],
    [ 0.35355339,  0.35355339,  0.70710678],
    [ 0.35355339,  0.35355339, -0.70710678],
    [ 0.35355339, -0.35355339, -0.70710678]]

func calcForce(fx: Double, fz: Double, mz: Double) -> Array<Double> {
    // get forces
    let ffx = fx * FORCE_MAX
    let ffz = fz * FORCE_MAX
    let mmz = mz * TORQUE_MAX
    
    // get motor speeds
    let m1 = MATRICE[0][0] * ffx + MATRICE[0][1] * ffz + MATRICE[0][2] * mmz
    let m2 = MATRICE[1][0] * ffx + MATRICE[1][1] * ffz + MATRICE[1][2] * mmz
    let m3 = MATRICE[2][0] * ffx + MATRICE[2][1] * ffz + MATRICE[2][2] * mmz
    let m4 = MATRICE[3][0] * ffx + MATRICE[3][1] * ffz + MATRICE[3][2] * mmz
    
    return [m1, m2, m3, m4]
}
