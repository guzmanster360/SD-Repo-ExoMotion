//
//  BLEManager.swift
//  Senior Design App
//
//  Created by Victor Guzman on 3/2/25.
//

import Foundation
import CoreBluetooth

struct DataIn: Codable {
    let seaTorque: Double
    let emgRaw: Double
    let accelX: Double
    let accelY: Double
    let accelZ: Double
    let gyroX: Double
    let gyroY: Double
    let gyroZ: Double
    let fsrPres: Double
}

class BLEManager: NSObject, CBCentralManagerDelegate, CBPeripheralDelegate {
    
    var centralManager: CBCentralManager!
    var esp32Peripheral: CBPeripheral?
    let serviceUUID = CBUUID(string: "12345678-1234-1234-1234-123456789abc")
    let characteristicUUID = CBUUID(string: "87654321-4321-4321-4321-abcdefabcdef")
    var connectionStatus: String = "Disconnected"
    
    var awsManager = AWSManager()
    var storageManager = StorageManager()
    
    var userID: String?
    var sessionID: String?
    var sessionDescription: String?


    override init() {
        super.init()
        centralManager = CBCentralManager(delegate: self, queue: nil)
    }

    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        if central.state == .poweredOn {
            centralManager.scanForPeripherals(withServices: [serviceUUID], options: nil)
        } else {
            print("Bluetooth is off")
        }
    }

    func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String : Any], rssi RSSI: NSNumber) {
        if peripheral.name == "ESP32_BLE" {
            esp32Peripheral = peripheral
            esp32Peripheral?.delegate = self
            centralManager.stopScan()
            centralManager.connect(esp32Peripheral!, options: nil)
        }
    }

    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        peripheral.discoverServices([serviceUUID])
    }

    func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
        if let services = peripheral.services {
            for service in services {
                peripheral.discoverCharacteristics([characteristicUUID], for: service)
            }
        }
    }

    func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?) {
        if let characteristics = service.characteristics {
            for characteristic in characteristics {
                if characteristic.uuid == characteristicUUID {
                    peripheral.setNotifyValue(true, for: characteristic)
                }
            }
        }
    }

    func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: Error?) {
        if let data = characteristic.value, let inputDataJson = String(data: data, encoding: .utf8) {
            print("Received: \(inputDataJson)")
            
            // parse Input data
            if let jsonData = inputDataJson.data(using: .utf8) {
                // ensure input compatibility
                do {
                    let sensorData = try JSONDecoder().decode(DataIn.self, from: jsonData)
                    print("SEA Torque N-m: \(sensorData.seaTorque)")
                    print("EMG Raw Data: \(sensorData.emgRaw)")
                    print("Acceleration X: \(sensorData.accelX)")
                    print("Acceleration Y: \(sensorData.accelY)")
                    print("Acceleration Z: \(sensorData.accelZ)")
                    print("Gyroscope X: \(sensorData.gyroX)")
                    print("Gyroscope Y: \(sensorData.gyroY)")
                    print("Gyroscope Z: \(sensorData.gyroZ)")
                    print("FSR Pressure: \(sensorData.fsrPres)")
                    
                    let fetchedResults = storageManager.retrieveUserData()
                        for userStored in fetchedResults {
                            if fetchedResults.count > 0 {
                                userID = userStored.value(forKey: "userID") as? String
                                sessionDescription = userStored.value(forKey: "sessionDescription") as? String
                                sessionID = userStored.value(forKey: "sessionID") as? String
                                
                                // combine all data into one JSON
                                let jsonPackage = DataOut(timestamp: (awsManager.getTimestamp()), userID: userID!, sessionID: sessionID!, sessionDescription: sessionDescription!, seaTorque: sensorData.seaTorque, emgRaw: sensorData.emgRaw, accelX: sensorData.accelX, accelY: sensorData.accelY, accelZ: sensorData.accelZ, gyroX: sensorData.gyroX, gyroY: sensorData.gyroY, gyroZ: sensorData.gyroZ, fsrPres: sensorData.fsrPres)
                            
                                if let jsonData = try? JSONEncoder().encode(jsonPackage) {
                                    let jsonString = String(data: jsonData, encoding: .utf8)
                                    awsManager.sendDataToKinesis(key: String(Int.random(in: 1...1000)), data: jsonString!)
                                    print("JSON Payload: \(jsonString!)")
                                }
                            }
                        }
                } catch {
                    print("JSON parsing error: \(error)")
                }
            }
        }
    }
    
    func centralManager(_ central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral, error: Error?) {
        print("Disconnected. Reconnecting...")
        centralManager.connect(peripheral, options: nil)
    }
    
    func startScanning() {
            connectionStatus = "Scanning..."
            centralManager.scanForPeripherals(withServices: nil, options: nil)
        }
}
