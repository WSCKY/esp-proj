#ifndef __MPU6500_REG_H
#define __MPU6500_REG_H

/* Registers */
#define MPU_REG_SMPDIV       (0x19)    /* Sample Rate Divider */
#define MPU_REG_CONFIG       (0x1A)    /* Configuration */
#define MPU_REG_GYRCFG       (0x1B)    /* Gyroscope Configuration */
#define MPU_REG_ACCCFG       (0x1C)    /* Accelerometer Configuration */
#define MPU_REG_ACFG_2       (0x1D)    /* Accelerometer Configuration 2 */
#define MPU_REG_SGNRST       (0x68)    /* Signal Path Reset */
#define MPU_ERG_PWRMGT       (0x6B)    /* Power Management 1 */
#define MPU_REG_WHOAMI       (0x75)    /* Who Am I */

#define MPU_REG_ACC_XOUT_H   (0x3B)    /* High byte of accelerometer X-axis data */
#define MPU_REG_ACC_XOUT_L   (0x3C)    /* Low byte of accelerometer X-axis data */
#define MPU_REG_ACC_YOUT_H   (0x3D)    /* High byte of accelerometer Y-axis data */
#define MPU_REG_ACC_YOUT_L   (0x3E)    /* Low byte of accelerometer Y-axis data */
#define MPU_REG_ACC_ZOUT_H   (0x3F)    /* High byte of accelerometer Z-axis data */
#define MPU_REG_ACC_ZOUT_L   (0x40)    /* Low byte of accelerometer Z-axis data */
#define MPU_REG_TEMP_OUT_H   (0x41)    /* High byte of the temperature sensor output */
#define MPU_REG_TEMP_OUT_L   (0x42)    /* Low byte of the temperature sensor output */
#define MPU_REG_GYR_XOUT_H   (0x43)    /* High byte of the X-axis gyroscope output */
#define MPU_REG_GYR_XOUT_L   (0x44)    /* Low byte of the X-axis gyroscope output */
#define MPU_REG_GYR_YOUT_H   (0x45)    /* High byte of the Y-axis gyroscope output */
#define MPU_REG_GYR_YOUT_L   (0x46)    /* Low byte of the Y-axis gyroscope output */
#define MPU_REG_GYR_ZOUT_H   (0x47)    /* High byte of the Z-axis gyroscope output */
#define MPU_REG_GYR_ZOUT_L   (0x48)    /* Low byte of the Z-axis gyroscope output */

#define MPU_REG_GYR_X_OFFS_H (0x13)    /* Bits 15 to 8 of the 16-bit offset of X gyroscope */
#define MPU_REG_GYR_X_OFFS_L (0x14)    /* Bits 7 to 0 of the 16-bit offset of X gyroscope */
#define MPU_REG_GYR_Y_OFFS_H (0x15)    /* Bits 15 to 8 of the 16-bit offset of Y gyroscope */
#define MPU_REG_GYR_Y_OFFS_L (0x16)    /* Bits 7 to 0 of the 16-bit offset of Y gyroscope */
#define MPU_REG_GYR_Z_OFFS_H (0x17)    /* Bits 15 to 8 of the 16-bit offset of Z gyroscope */
#define MPU_REG_GYR_Z_OFFS_L (0x18)    /* Bits 7 to 0 of the 16-bit offset of Z gyroscope */

/* Register bits */
#define GYR_FS_SEL_Msk       (0x18)    /* Gyro Full Scale Select, <Gyroscope Configuration> */
#define GYR_FS_SEL_Pos       (3)

#define ACC_FS_SEL_Msk       (0x18)    /* Accel Full Scale Select, <Accelerometer Configuration> */
#define ACC_FS_SEL_Pos       (3)

#define DLPF_BYPASS_Msk      (0x03)    /* Used to bypass DLPF, <Gyroscope Configuration> */
#define DLPF_BYP_Disable     (0x00)
#define DLPF_BYP_BW_8800     (0x01)
#define DLPF_BYP_BW_3600     (0x02)

#define DLPF_CONFIG_Msk      (0x07)    /* DLPF Configuration for Gyro and Temperation, <Configuration> */
#define DLPF_CFG_BW_250Hz    (0x00)
#define DLPF_CFG_BW_184Hz    (0x01)
#define DLPF_CFG_BW_92Hz     (0x02)
#define DLPF_CFG_BW_41Hz     (0x03)
#define DLPF_CFG_BW_20Hz     (0x04)
#define DLPF_CFG_BW_10Hz     (0x05)
#define DLPF_CFG_BW_5Hz      (0x06)
#define DLPF_CFG_BW_3600Hz   (0x07)

/* mpu6500 configuration */
typedef enum {
	gyr_fs_250dps = 0,
	gyr_fs_500dps = 1,
	gyr_fs_1000dps = 2,
	gyr_fs_2000dps = 3,
} gyr_fs_t;

typedef enum {
	acc_fs_2g = 0,
	acc_fs_4g = 1,
	acc_fs_8g = 2,
	acc_fs_16g = 3,
} acc_fs_t;

typedef enum {
	dlpf_bw_250hz = 0,
	dlpf_bw_184hz = 1,
	dlpf_bw_92hz = 2,
	dlpf_bw_41hz = 3,
	dlpf_bw_20hz = 4,
	dlpf_bw_10hz = 5,
	dlpf_bw_5hz = 6,
	dlpf_bw_3600hz = 7,
	dlpf_byp_bw_8800 = 8,
	dlpf_byp_bw_3600 = 9,
} gyr_dlpf_t;

#endif /* __MPU6500_REG_H */

