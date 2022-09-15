/*
 * ModbusRegister.h
 *
 *  Created on: 13.2.2019
 *      Author: keijo
 */

#ifndef MODBUSREGISTER_H_
#define MODBUSREGISTER_H_

#include "ModbusMaster.h"

class ModbusRegister {
public:
	ModbusRegister(ModbusMaster *master, int address, bool holdingRegister = true);
	ModbusRegister(const ModbusRegister &)  = delete;
	virtual ~ModbusRegister();
	int read();
	void write(int value);
private:
	ModbusMaster *m;
	int addr;
	bool hr;
};

#endif /* MODBUSREGISTER_H_ */
