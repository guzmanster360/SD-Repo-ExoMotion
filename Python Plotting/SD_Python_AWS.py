import boto3
import json
import pandas as pd
import streamlit as st
import plotly.express as px
import time
from datetime import datetime
from collections import deque

class AWSManager():

    def __init__(self, region, stream_name, bucket, prefix):
        self.region = region
        self.stream_name = stream_name
        self.bucket = bucket
        self.prefix = prefix

        self.kinesis_client = boto3.client("kinesis", region_name= self.region)
        self.s3_client = boto3.client("s3", region_name=region)
    # AWS Client
    

    def list_shards(self):
        """Retrieve all shard IDs from the Kinesis stream."""
        shard_ids = []
        response = self.kinesis_client.list_shards(StreamName=self.stream_name)

        while response:
            shard_ids.extend([shard["ShardId"] for shard in response["Shards"]])
            if "NextToken" in response:
                response =self.kinesis_client.list_shards(NextToken=response["NextToken"])
            else:
                break

        return shard_ids

    def get_records(self, shardID):
        # Get the shard iterator
        shard_iterator_response = self.kinesis_client.get_shard_iterator(
        StreamName = self.stream_name,
        ShardId = shardID,
        ShardIteratorType = "LATEST"
        )
        shard_iterator = shard_iterator_response["ShardIterator"]

        records_response = self.kinesis_client.get_records(ShardIterator=shard_iterator, Limit=100)
        records = [json.loads(record["Data"].decode("utf-8")) for record in records_response["Records"]]
        return pd.DataFrame(records) if records else pd.DataFrame()

    def get_all_records(self):
        """Iterate through all shards and fetch records."""
        all_data = []
        shard_ids = self.list_shards()

        for shard_id in shard_ids:
            df = self.get_records(shard_id)
            print(df)
            if not df.empty:
                all_data.append(df)

        return pd.concat(all_data, ignore_index=True) if all_data else pd.DataFrame()

    def list_s3_files(self, bucket, prefix):
        """List all files in the S3 bucket under the specified prefix."""
        response = self.s3_client.list_objects_v2(Bucket=bucket, Prefix = prefix)
        return [obj["Key"] for obj in response.get("Contents", [])]

    def download_s3_file(self, bucket, file_key):
        """Download a file from S3 and parse it as a list of JSON objects."""
        response = self.s3_client.get_object(Bucket=bucket, Key=file_key)
        data = response["Body"].read().decode("utf-8")

        # Ensure proper separation of JSON objects before decoding
        formatted_data = data.replace("}{", "}\n{")  # ✅ Insert newlines between concatenated JSON objects
        try:
            return [json.loads(line) for line in formatted_data.strip().split("\n") if line]  # ✅ Properly handles multiple JSON objects
        except json.JSONDecodeError as e:
            print(f"JSON Decode Error in {file_key}: {e}")
            print(f"First 500 characters of file: {formatted_data[:500]}")  # Debugging hint
            return []

    def get_s3_data(self, prefix):
        """Fetch latest S3 data and return as DataFrame."""
        files = self.list_s3_files(self.bucket, prefix)
        if not files:
            return pd.DataFrame()
        latest_file = sorted(files)[-1]  # Get the most recent file
        data = self.download_s3_file(self.bucket, latest_file)
        # Assuming df is your DataFrame    
        return pd.DataFrame(data)
