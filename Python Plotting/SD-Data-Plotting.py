import boto3
import json
import pandas as pd
import streamlit as st
import plotly.express as px
import time
from datetime import datetime
from collections import deque
import SD_Python_AWS

# DATA REFRENCE
# struct DataOut: Codable {
#     let timestamp: String
#     let userID: String
#     let sessionID: String
#     let sessionDescription: String
#     let seaTorque: Double
#     let emgRaw: Double
#     let accelX: Double
#     let accelY: Double
#     let accelZ: Double
#     let gyroX: Double
#     let gyroY: Double
#     let gyroZ: Double
#     let fsrPres_1: Double
#     let fsrPres_2: Double
#     let fsrPres_3: Double
#     let fsrPres_4: Double
#     let fsrPres_5: Double
#     let fsrPres_6: Double
# }


# AWS Kinesis setup
STREAM_NAME = "SD_1"
REGION = "us-east-2"
# AWS S3 setup
S3_BUCKET = "seniordesign20251"
S3_PREFIX = "data/"

users_data = {}

aws_manager = SD_Python_AWS.AWSManager(region=REGION, stream_name=STREAM_NAME, bucket=S3_BUCKET)

storage_tail = 1000
live_tail = 1000

# Streamlit UI
st.set_page_config(page_title="ExoMotion Dashboard", layout="wide")

st.title("📊 ExoMotion Data Collection Dashboard")

# Data
if "sea_live_data" not in st.session_state:
    st.session_state.sea_live_data = pd.DataFrame(columns=["timestamp", "seaTorque"])
if "sea_stored_data" not in st.session_state:
    st.session_state.sea_stored_data = pd.DataFrame(columns=["timestamp", "seaTorque"])
if "last_sea_update" not in st.session_state:
    st.session_state.last_sea_update = time.time()

if "emg_live_data" not in st.session_state:
    st.session_state.emg_live_data = pd.DataFrame(columns=["timestamp", "emgRaw"])
if "emg_stored_data" not in st.session_state:
    st.session_state.emg_stored_data = pd.DataFrame(columns=["timestamp", "emgRaw"])
if "last_emg_update" not in st.session_state:
    st.session_state.last_emg_update = time.time()

# SETUP ACCEL DATA FRAME
if "accelXYZ_live_data" not in st.session_state:
    st.session_state.accelXYZ_live_data = pd.DataFrame(columns=["timestamp", "accelX", "accelY", "accelZ"])
if "accelXYZ_stored_data" not in st.session_state:
    st.session_state.accelXYZ_stored_data = pd.DataFrame(columns=["timestamp", "accelX", "accelY", "accelZ"])
if "last_accelXYZ_update" not in st.session_state:
    st.session_state.last_accelXYZ_update = time.time()

# SETUP GYRO DATA FRAME 
if "gyroXYZ_live_data" not in st.session_state:
    st.session_state.gyroXYZ_live_data = pd.DataFrame(columns=["timestamp", "gyroX", "gyroY", "gyroZ"])
if "gyroXYZ_stored_data" not in st.session_state:
    st.session_state.gyroXYZ_stored_data = pd.DataFrame(columns=["timestamp", "gyroX", "gyroY", "gyroZ"])
if "last_gyroX_update" not in st.session_state:
    st.session_state.last_gyroXYZ_update = time.time()

if "fsr_live_data" not in st.session_state:
    st.session_state.fsr_live_data = pd.DataFrame(columns=["timestamp", "fsrPres_1", "fsrPres_2", "fsrPres_3", "fsrPres_4", "fsrPres_5", "fsrPres_6"])
if "fsr_stored_data" not in st.session_state:
    st.session_state.fsr_stored_data = pd.DataFrame(columns=["timestamp", "fsrPres_1", "fsrPres_2", "fsrPres_3", "fsrPres_4", "fsrPres_5", "fsrPres_6"])
if "last_fsr_update" not in st.session_state:
    st.session_state.last_fsr_update = time.time()


user_uuids = [path.split('/')[0] for path in aws_manager.list_s3_user_files(S3_PREFIX)]

for i in user_uuids:
    sessionPrefix = f"{S3_PREFIX}{i}/"
    session_uuids = [path.split('/')[0] for path in aws_manager.list_s3_user_files(sessionPrefix)]
    sessions = []
    for j in session_uuids:
        if j not in sessions:
            sessions.append(j)
    users_data[i] = sessions


with st.sidebar:
    selected__user = st.selectbox("Select User:", list(users_data.keys()), key=f'user_box')
    selected__session = st.selectbox("Select Session:", users_data[selected__user], key=f'session_box')

col1, col2 = st.columns(2)  # ✅ Create two columns for side-by-side plots
with col1:
    # SEA Live Data Setup
    st.subheader("🔵 Live SEA Data")
    chart_seaL = st.empty()
    # EMG Live Data Setup
    st.subheader("🔴 Live EMG Data")
    chart_emgL = st.empty()
    # Accel. Live Data Setup
    st.subheader("🟢 Live Accel. XYZ Data")
    chart_accelXYZL = st.empty()
    # Gyro Live Data Setup
    st.subheader("🟠 Live Gyro. XYZ Data")
    chart_gyroXYZL = st.empty()
    # FSR Live Data Setup
    st.subheader("🟡 Live FSR Data")
    chart_fsrL = st.empty()
with col2:
    # SEA Stored Data Setup
    st.subheader("🔵 Stored SEA Data")
    chart_seaS = st.empty()
    # EMG Stored Data Setup
    st.subheader("🔴 Stored EMG Data")
    chart_emgS = st.empty()
    # Accel. Stored Data Setup
    st.subheader("🟢 Stored Accel. XYZ Data")
    chart_accelXYZS = st.empty()
    # Gyro Stored Data Setup
    st.subheader("🟠 Stored Gyro. XYZ Data")
    chart_gyroXYZS = st.empty()
    # FSR Live Data Setup
    st.subheader("🟡 Stored FSR Data")
    chart_fsrS = st.empty()

while True:
    all_data = aws_manager.get_all_records()
    print(all_data)
    # Assuming df is your DataFrame
    all_s3_data = aws_manager.get_s3_data(f"{S3_PREFIX}{selected__user}/{selected__session}/")
    all_s3_data = all_s3_data.sort_values(by="timestamp", ascending=True)  # Sort in ascending order (oldest first)

    # SEA DATA BLOCK
    # SEA LIVE DATA BLOCK
    if not all_data.empty:
        st.session_state.sea_live_data = pd.concat([st.session_state.sea_live_data, all_data]).tail(live_tail)
        st.session_state.emg_live_data["emgRaw"] = st.session_state.emg_live_data["emgRaw"].astype('object')
        st.session_state.last_sea_update = time.time()  # Update timestamp

    if time.time() - st.session_state.last_sea_update > 5:
        st.session_state.sea_live_data = pd.DataFrame(columns=["timestamp", "seaTorque"])

    if not st.session_state.sea_live_data.empty:
        fig_seaL = px.line(st.session_state.sea_live_data, x="timestamp", y="seaTorque", title="📈 Real-time SEA Data", color_discrete_sequence=["blue"])
        fig_seaL.update_xaxes(title_text="Time")
        fig_seaL.update_yaxes(title_text="SEA Torque Data (N-m)")
    else:
        fig_seaL = px.line(title="📈 Waiting for SEA Data...")

    chart_seaL.plotly_chart(fig_seaL, use_container_width=True)

    #SEA S3 BLOCK
    if not all_s3_data.empty:
        st.session_state.sea_stored_data = pd.concat([st.session_state.sea_stored_data, all_s3_data])
        st.session_state.sea_stored_data["seaTorque"] = st.session_state.sea_stored_data["seaTorque"].astype('object')
    else:
        st.session_state.sea_stored_data = pd.DataFrame(columns=["timestamp", "seaTorque"])

    if not st.session_state.sea_stored_data.empty:
        fig_seaS = px.line(st.session_state.sea_stored_data, x="timestamp", y="seaTorque", title="📈 SEA Stored Data", color_discrete_sequence=["blue"])
        fig_seaS.update_xaxes(title_text="Time")
        fig_seaS.update_yaxes(title_text="Torque Data (N-m)")
    else:
        fig_seaS = px.line(title="📈 No Stored SEA Data Available...")

    chart_seaS.plotly_chart(fig_seaS, use_container_width=True)

    # EMG DATA BLOCK
    # EMG LIVE DATA BLOCK
    if not all_data.empty:
        st.session_state.emg_live_data = pd.concat([st.session_state.emg_live_data, all_data]).tail(live_tail)
        st.session_state.emg_live_data["emgRaw"] = st.session_state.emg_live_data["emgRaw"].astype('object')
        st.session_state.last_emg_update = time.time()  # Update timestamp

    if time.time() - st.session_state.last_emg_update > 5:
        st.session_state.emg_live_data = pd.DataFrame(columns=["timestamp", "emgRaw"])

    if not st.session_state.emg_live_data.empty:
        fig_emgL = px.line(st.session_state.emg_live_data, x="timestamp", y="emgRaw", title="📊 Real-time EMG Data", color_discrete_sequence=["red"])
        fig_emgL.update_xaxes(title_text="Time")
        fig_emgL.update_yaxes(title_text="EMG Signal Value")
    else:
        fig_emgL = px.line(title="📈 Waiting for EMG Data...")  # Placeholder chart
    chart_emgL.plotly_chart(fig_emgL, use_container_width=True)

    # S3 EMG DATA BLOCK
    if not all_s3_data.empty:
        st.session_state.emg_stored_data = pd.concat([st.session_state.emg_stored_data, all_s3_data])
        st.session_state.emg_stored_data["emgRaw"] = st.session_state.emg_stored_data["emgRaw"].astype('object')
    else:
        st.session_state.emg_stored_data = pd.DataFrame(columns=["timestamp", "emgRaw"])

    if not st.session_state.emg_stored_data.empty:
        fig_emgS = px.line(st.session_state.emg_stored_data, x="timestamp", y="emgRaw", title="📊 EMG Stored Data", color_discrete_sequence=["red"])
        fig_emgS.update_xaxes(title_text="Time")
        fig_emgS.update_yaxes(title_text="Stored Signal Value")
    else:
        fig_emgS = px.line(title="📈 No Stored EMG Data Available...")

    chart_emgS.plotly_chart(fig_emgS, use_container_width=True)

    # ACCEL XYZ DATA BLOCK
    # ACCL XYZ LIVE DATA BLOCK
    if not all_data.empty:
        st.session_state.accelXYZ_live_data = pd.concat([st.session_state.accelXYZ_live_data, all_data]).tail(live_tail)
        st.session_state.accelXYZ_live_data["accelX"] = st.session_state.accelXYZ_live_data["accelX"].astype('object')
        st.session_state.accelXYZ_live_data["accelY"] = st.session_state.accelXYZ_live_data["accelY"].astype('object')
        st.session_state.accelXYZ_live_data["accelZ"] = st.session_state.accelXYZ_live_data["accelZ"].astype('object')
        st.session_state.last_accelXYZ_update = time.time()  # Update timestamp

    if time.time() - st.session_state.last_accelXYZ_update > 5:
        st.session_state.accelXYZ_live_data = pd.DataFrame(columns=["timestamp", "accelX", "accelY", "accelZ"])

    if not st.session_state.accelXYZ_live_data.empty:
        fig_accelXYZ_L = px.line(st.session_state.accelXYZ_live_data, x="timestamp", 
        y=["accelX", "accelY", "accelZ"], 
        title="📈 Real-time XYZ Acceleration Data",
        color_discrete_sequence=["red", "green", "blue"]
        )
        fig_accelXYZ_L.update_xaxes(title_text="Time")
        fig_accelXYZ_L.update_yaxes(title_text="XYZ Acceleration Data (m/s/s)")
    else:
        fig_accelXYZ_L = px.line(title="📈 Waiting for IMU XYZ Acceleration Data...")

    chart_accelXYZL.plotly_chart(fig_accelXYZ_L, use_container_width=True)

    #ACCEL XYZ S3 BLOCK
    if not all_s3_data.empty:
        all_s3_data.reset_index(drop=True, inplace=True)
        st.session_state.accelXYZ_stored_data = pd.concat([st.session_state.accelXYZ_stored_data, all_s3_data])
        st.session_state.accelXYZ_stored_data["accelX"] = st.session_state.accelXYZ_stored_data["accelX"].astype('object')
        st.session_state.accelXYZ_stored_data["accelY"] = st.session_state.accelXYZ_stored_data["accelY"].astype('object')
        st.session_state.accelXYZ_stored_data["accelZ"] = st.session_state.accelXYZ_stored_data["accelZ"].astype('object')
    else:
        st.session_state.accelXYZ_stored_data = pd.DataFrame(columns=["timestamp", "accelX", "accelY", "accelZ"])

    if not st.session_state.accelXYZ_stored_data.empty:
        fig_accelXYZ_S = px.line(st.session_state.accelXYZ_stored_data, x="timestamp", 
            y=["accelX", "accelY", "accelZ"], 
            title="📊 Stored XYZ Acceleration Data", 
            color_discrete_sequence=["red", "green", "blue"]
        )
        fig_accelXYZ_S.update_xaxes(title_text="Time")
        fig_accelXYZ_S.update_yaxes(title_text="Acceleration Data (m/s/s)")
    else:
        fig_accelXYZ_S = px.line(title="📈 No Stored XYZ Acceleration Data Available...")

    chart_accelXYZS.plotly_chart(fig_accelXYZ_S, use_container_width=True)

    # GYRO XYZ DATA BLOCK
    # GYRO XYZ LIVE DATA BLOCK
    if not all_data.empty:
        st.session_state.gyroXYZ_live_data = pd.concat([st.session_state.gyroXYZ_live_data, all_data]).tail(live_tail)
        st.session_state.gyroXYZ_live_data["gyroX"] = st.session_state.gyroXYZ_live_data["accelX"].astype('object')
        st.session_state.gyroXYZ_live_data["gyroY"] = st.session_state.gyroXYZ_live_data["accelY"].astype('object')
        st.session_state.gyroXYZ_live_data["gyroZ"] = st.session_state.gyroXYZ_live_data["accelZ"].astype('object')
        st.session_state.last_gyroXYZ_update = time.time()  # Update timestamp

    if time.time() - st.session_state.last_gyroXYZ_update > 5:
        st.session_state.gyroXYZ_live_data = pd.DataFrame(columns=["timestamp", "gyroX", "gyroY", "gyroZ"])

    if not st.session_state.gyroXYZ_live_data.empty:
        fig_gyroXYZ_L = px.line(st.session_state.gyroXYZ_live_data, x="timestamp", 
        y=["gyroX", "gyroY", "gyroZ"], 
        title="📈 Real-time XYZ Gyroscope Data", 
        color_discrete_sequence=["red", "green", "blue"]
        )
        fig_gyroXYZ_L.update_xaxes(title_text="Time")
        fig_gyroXYZ_L.update_yaxes(title_text="XYZ Gyroscope Data (m/s/s)")
    else:
        fig_gyroXYZ_L = px.line(title="📈 Waiting for IMU XYZ Gyroscope Data...")

    chart_gyroXYZL.plotly_chart(fig_gyroXYZ_L, use_container_width=True)

    #GYRO XYZ S3 BLOCK
    if not all_s3_data.empty:
        all_s3_data.reset_index(drop=True, inplace=True)
        st.session_state.gyroXYZ_stored_data = pd.concat([st.session_state.gyroXYZ_stored_data, all_s3_data])
        st.session_state.gyroXYZ_stored_data["gyroX"] = st.session_state.gyroXYZ_stored_data["gyroX"].astype('object')
        st.session_state.gyroXYZ_stored_data["gyroY"] = st.session_state.gyroXYZ_stored_data["gyroY"].astype('object')
        st.session_state.gyroXYZ_stored_data["gyroZ"] = st.session_state.gyroXYZ_stored_data["gyroZ"].astype('object')
    else:
        st.session_state.gyroXYZ_stored_data = pd.DataFrame(columns=["timestamp", "gyroX", "gyroY", "gyroZ"])

    if not st.session_state.gyroXYZ_stored_data.empty:
        fig_gyroXYZ_S = px.line(st.session_state.gyroXYZ_stored_data, x="timestamp", 
            y=["gyroX", "gyroY", "gyroZ"], 
            title="📊 Stored XYZ Gyroscope Data", 
            color_discrete_sequence=["blue", "green", "red"]
        )
        fig_gyroXYZ_S.update_xaxes(title_text="Time")
        fig_gyroXYZ_S.update_yaxes(title_text="Gyroscope Data (rad/s)")
    else:
        fig_gyroXYZ_S = px.line(title="📈 No Stored XYZ Gyroscope Data Available...")

    chart_gyroXYZS.plotly_chart(fig_gyroXYZ_S, use_container_width=True)

    # FSR DATA BLOCK
    # FSR LIVE DATA BLOCK
    if not all_data.empty:
        st.session_state.fsr_live_data = pd.concat([st.session_state.fsr_live_data, all_data]).tail(live_tail)
        st.session_state.fsr_live_data["fsrPres_1"] = st.session_state.fsr_live_data["fsrPres_1"].astype('object')
        st.session_state.fsr_live_data["fsrPres_2"] = st.session_state.fsr_live_data["fsrPres_2"].astype('object')
        st.session_state.fsr_live_data["fsrPres_3"] = st.session_state.fsr_live_data["fsrPres_3"].astype('object')
        st.session_state.fsr_live_data["fsrPres_4"] = st.session_state.fsr_live_data["fsrPres_4"].astype('object')
        st.session_state.fsr_live_data["fsrPres_5"] = st.session_state.fsr_live_data["fsrPres_5"].astype('object')
        st.session_state.fsr_live_data["fsrPres_6"] = st.session_state.fsr_live_data["fsrPres_6"].astype('object')
        st.session_state.fsr_live_data = time.time()  # Update timestamp

    if time.time() - st.session_state.last_fsr_update > 5:
        st.session_state.fsr_live_data = pd.DataFrame(columns=["timestamp", "fsrPres_1", "fsrPres_2", "fsrPres_3", "fsrPres_4", "fsrPres_5", "fsrPres_6"])

    if not st.session_state.fsr_live_data.empty:
        fig_fsrL = px.line(st.session_state.fsr_live_data, x="timestamp", 
        y=["fsrPres_1", "fsrPres_2", "fsrPres_3", "fsrPres_4", "fsrPres_5", "fsrPres_6"], 
        title="📊 Real-time FSR Data", 
        color_discrete_sequence=["red", "green", "yellow", "orange", "blue", "purple"]
        )
        fig_fsrL.update_xaxes(title_text="Time")
        fig_fsrL.update_yaxes(title_text="FSR Signal Value N")
    else:
        fig_fsrL = px.line(title="📈 Waiting for FSR Data...")  # Placeholder chart
    chart_fsrL.plotly_chart(fig_fsrL, use_container_width=True)

    # S3 FSR DATA BLOCK
    if not all_s3_data.empty:
        st.session_state.fsr_stored_data = pd.concat([st.session_state.fsr_stored_data, all_s3_data])
        st.session_state.fsr_stored_data["fsrPres_1"] = st.session_state.fsr_stored_data["fsrPres_1"].astype('object')
        st.session_state.fsr_stored_data["fsrPres_2"] = st.session_state.fsr_stored_data["fsrPres_2"].astype('object')
        st.session_state.fsr_stored_data["fsrPres_3"] = st.session_state.fsr_stored_data["fsrPres_3"].astype('object')
        st.session_state.fsr_stored_data["fsrPres_4"] = st.session_state.fsr_stored_data["fsrPres_4"].astype('object')
        st.session_state.fsr_stored_data["fsrPres_5"] = st.session_state.fsr_stored_data["fsrPres_5"].astype('object')
        st.session_state.fsr_stored_data["fsrPres_6"] = st.session_state.fsr_stored_data["fsrPres_6"].astype('object')

    else:
        st.session_state.fsr_stored_data = pd.DataFrame(columns=["timestamp", "fsrPres_1", "fsrPres_2", "fsrPres_3", "fsrPres_4", "fsrPres_5", "fsrPres_6"])

    if not st.session_state.fsr_stored_data.empty:
        fig_fsrS = px.line(st.session_state.fsr_stored_data, x="timestamp", 
        y=["fsrPres_1", "fsrPres_2", "fsrPres_3", "fsrPres_4", "fsrPres_5", "fsrPres_6"], 
        title="📊 FSR Stored Data", 
        color_discrete_sequence=["red", "green", "yellow", "orange", "blue", "purple"]
        )
        fig_fsrS.update_xaxes(title_text="Time")
        fig_fsrS.update_yaxes(title_text="Force (N)")
    else:
        fig_fsrS = px.line(title="📈 No Stored FSR Data Available...")

    chart_fsrS.plotly_chart(fig_fsrS, use_container_width=True)

    time.sleep(0.01)  # Update Time