/****************************************************************************
 * boards/arm/stm32/stm32f103-minimum/src/stm32_6050.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <debug.h>

#include <nuttx/i2c/i2c_master.h>
#include <nuttx/mtd/mtd.h>
#include <nuttx/fs/fs.h>
#include <nuttx/fs/nxffs.h>
#include <nuttx/sensors/mpu60x0.h>

#include "stm32_i2c.h"
#include "stm32f103_minimum.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32_mpu6050
 *
 * Description:
 *   Initialize and configure the AT24 serial EEPROM
 *
 ****************************************************************************/

int stm32_mpu6050(void)
{
  struct i2c_master_s *i2c;

  finfo("Initialize I2C%d\n", 2);
  i2c = stm32_i2cbus_initialize(2);
  if (!i2c)
  {
    ferr("ERROR: Failed to initialize I2C%d\n", 2);
    return -ENODEV;
  }

  i2c_register(i2c, 2);

  struct mpu_config_s *config = NULL;
  config = kmm_zalloc(sizeof(struct mpu_config_s));

  config->i2c = i2c;
  config->addr = 0x68;

  mpu60x0_register("/dev/imu0", config);

  return OK;
}
