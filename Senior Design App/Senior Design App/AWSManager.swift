//
//  File.swift
//  Senior Design App
//
//  Created by Victor Guzman on 3/27/25.
//

import UIKit
import AWSKinesis
import AWSCore
import AWSS3

struct DataOut: Codable {
    let timestamp: String
    let userID: String
    let sessionID: String
    let sessionDescription: String
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

class AWSManager {
    
    let kinesis = AWSKinesis.default()
    
    let bucketName = "seniordesign20251"
    let prefix = "data"  // Ensure it has a trailing slash "/"

    func sendDataToKinesis(key: String, data: String) {
        let record = AWSKinesisPutRecordInput()
        record!.streamName = "SD_1"  // Replace with your stream name
        record!.data = data.data(using: .utf8)
        record!.partitionKey = key

        kinesis.putRecord(record!) { response, error in
            if let error = error {
                print("Error sending data: \(error)")
            } else {
                print("Data sent successfully: \(response!)")
            }
        }
    }
    
    func getTimestamp() -> String{
        let timestamp = Date()
        let formatter = ISO8601DateFormatter()
        formatter.formatOptions = [.withInternetDateTime, .withFractionalSeconds] // Includes milliseconds
        return formatter.string(from: timestamp)
    }
    
    func createS3Folder(userID: String, sessionID: String) {
        let s3 = AWSS3.default()
        let putObjectRequest = AWSS3PutObjectRequest()!
        let folderName = prefix + "/" + userID + "/" + sessionID + "/"
        
        putObjectRequest.bucket = bucketName
        putObjectRequest.key = folderName// Folder name with trailing slash
        putObjectRequest.body = nil  // No content, just creates a folder
        
        s3.putObject(putObjectRequest).continueWith { task in
            if let error = task.error {
                print("Error creating folder: \(error.localizedDescription)")
            } else {
                print("Folder '\(folderName)' created successfully in bucket '\(self.bucketName)'")
            }
            return nil
        }
    }
}
