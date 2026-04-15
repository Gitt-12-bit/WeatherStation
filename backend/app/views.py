"""
Flask Documentation:     https://flask.palletsprojects.com/
Jinja2 Documentation:    https://jinja.palletsprojects.com/
Werkzeug Documentation:  https://werkzeug.palletsprojects.com/
This file creates your application.
"""

import site 

from app import app, Config, mongo, Mqtt
from flask import escape, render_template, request, jsonify, send_file, redirect, make_response, send_from_directory 
from json import dumps, loads 
from werkzeug.utils import secure_filename
from datetime import datetime, timedelta, timezone
from os import getcwd
from os.path import join, exists
from time import time, ctime
from math import floor


#####################################
#   Routing for your application    #
#####################################

# 1. CREATE ROUTE FOR '/api/set/combination'
@app.route('/api/set/combination', methods=['POST'])
def set_combination():
    if request.method == "POST":
        passcode = request.form.get('passcode')
        print("Passcode received:", passcode)
        if passcode and passcode.isdigit() and len(passcode) == 4:
            result = mongo.set_passcode(passcode)
            print("Mongo result:", result)
            if result:
                return jsonify({"status": "complete", "data": "complete"})
    return jsonify({"status": "failed", "data": "failed"})

# 2. CREATE ROUTE FOR '/api/check/combination'
@app.route('/api/check/combination', methods=['POST'])
def check_combination():
    if request.method == "POST":
        passcode = request.form.get('passcode')
        if mongo.check_passcode(passcode):
            return jsonify({"status": "complete", "data": "complete"})
    return jsonify({"status": "failed", "data": "failed"})

# 3. CREATE ROUTE FOR '/api/update'
@app.route('/api/update', methods=['POST'])
def update_radar():
    if request.method == "POST":
        print("Raw data received:", request.data)     
        print("Content-Type:", request.content_type)   
        json_data = request.get_json()
        print("Parsed JSON:", json_data)
        if json_data:
            json_data['timestamp'] = int(time())
            Mqtt.publish("620171757", dumps(json_data)) 
            if mongo.insert_radar_data(json_data):
                return jsonify({"status": "complete", "data": "complete"})
    return jsonify({"status": "failed", "data": "failed"})

# 4. CREATE ROUTE FOR '/api/reserve/<start>/<end>'
@app.route('/api/reserve/<start>/<end>', methods=['GET'])
def get_reserve(start, end):
    if request.method == "GET":
        data = mongo.get_radar_range(start, end)
        if data:
            return jsonify({"status": "found", "data": data})
    return jsonify({"status": "failed", "data": []})

# 5. CREATE ROUTE FOR '/api/avg/<start>/<end>'
@app.route('/api/avg/<start>/<end>', methods=['GET'])
def get_avg(start, end):
    if request.method == "GET":
        avg_list = mongo.get_reserve_average(start, end)
        if avg_list:
            return jsonify({"status": "found", "data": avg_list[0]['average']})
    return jsonify({"status": "failed", "data": 0})


#####################################
#   WEATHER STATION ROUTES          #
#####################################

# 6. RECEIVE SENSOR DATA POSTED DIRECTLY FROM ESP32 VIA HTTP
#    ESP32 posts JSON to: http://<backendIP>:8080/api/weather/update
#    Payload: {"temp": 27.5, "hum": 77.1, "pres": 991.0, "soil": 24}
@app.route('/api/weather/update', methods=['POST'])
def weather_update():
    if request.method == "POST":
        json_data = request.get_json()
        print("[WEATHER] Received:", json_data)
        if json_data:
            # Add Unix timestamp — same pattern as radar
            json_data['timestamp'] = int(time())

            # Publish to MQTT so any subscribers also get it
            Mqtt.publish("weatherstation/data", dumps(json_data))

            # Save to MongoDB weather collection
            result = mongo.insert_weather_data(json_data)
            if result:
                return jsonify({"status": "complete", "data": "complete"})
    return jsonify({"status": "failed", "data": "failed"})

# 7. GET ALL WEATHER READINGS BETWEEN TWO UNIX TIMESTAMPS
#    GET /api/weather/<start>/<end>
@app.route('/api/weather/<start>/<end>', methods=['GET'])
def get_weather(start, end):
    if request.method == "GET":
        data = mongo.get_weather_range(start, end)
        if data:
            return jsonify({"status": "found", "data": data})
    return jsonify({"status": "failed", "data": []})

# 8. GET THE MOST RECENT WEATHER READING
#    GET /api/weather/latest
@app.route('/api/weather/latest', methods=['GET'])
def get_latest_weather():
    if request.method == "GET":
        data = mongo.get_latest_weather()
        if data:
            return jsonify({"status": "found", "data": data})
    return jsonify({"status": "failed", "data": {}})

# 9. GET AVERAGE OF A SPECIFIC FIELD BETWEEN TWO UNIX TIMESTAMPS
#    GET /api/weather/avg/<field>/<start>/<end>
#    Example: /api/weather/avg/temp/1700000000/1700099999
@app.route('/api/weather/avg/<field>/<start>/<end>', methods=['GET'])
def get_weather_avg(field, start, end):
    valid_fields = ["temp", "hum", "pres", "soil"]
    if field not in valid_fields:
        return jsonify({"status": "failed", "data": "invalid field"})
    if request.method == "GET":
        avg = mongo.get_weather_average(field, start, end)
        if avg is not None:
            return jsonify({"status": "found", "data": avg})
    return jsonify({"status": "failed", "data": 0})


###############################################################
# The functions below should be applicable to all Flask apps. #
###############################################################

@app.route('/api/file/get/<filename>', methods=['GET']) 
def get_images(filename):   
    if request.method == "GET":
        directory = join(getcwd(), Config.UPLOADS_FOLDER) 
        filePath  = join(getcwd(), Config.UPLOADS_FOLDER, filename) 
        if exists(filePath):        
            return send_from_directory(directory, filename)
        return jsonify({"status": "file not found"}), 404


@app.route('/api/file/upload', methods=["POST"])  
def upload():
    if request.method == "POST": 
        file     = request.files['file']
        filename = secure_filename(file.filename)
        file.save(join(getcwd(), Config.UPLOADS_FOLDER, filename))
        return jsonify({"status": "File upload successful", "filename": f"{filename}"})


@app.after_request
def add_header(response):
    response.headers['X-UA-Compatible'] = 'IE=Edge,chrome=1'
    response.headers['Cache-Control'] = 'public, max-age=0'
    return response

@app.errorhandler(405)
def page_not_found(error):
    return jsonify({"status": 404}), 404

@app.route('/api/weather/one/<timestamp>', methods=['GET'])
def get_weather_one(timestamp):
    data = mongo.get_weather_by_timestamp(timestamp)
    if data:
        return jsonify({"status": "found", "data": data})
    return jsonify({"status": "failed", "data": {}})


# ── UPDATE: update a document by timestamp ──
# PUT /api/weather/update/<timestamp>
# Body: {"temp": 28.0, "hum": 80.0}  — any subset of fields
@app.route('/api/weather/update/<timestamp>', methods=['PUT'])
def update_weather(timestamp):
    json_data = request.get_json()
    if not json_data:
        return jsonify({"status": "failed", "data": "no body"})

    # Prevent overwriting the timestamp itself
    json_data.pop("timestamp", None)

    result = mongo.update_weather_data(timestamp, json_data)
    if result:
        return jsonify({"status": "complete", "data": result})
    return jsonify({"status": "failed", "data": "not found"})


# ── DELETE: remove a single document by timestamp ──
# DELETE /api/weather/delete/<timestamp>
@app.route('/api/weather/delete/<timestamp>', methods=['DELETE'])
def delete_weather(timestamp):
    count = mongo.delete_weather_data(timestamp)
    if count:
        return jsonify({"status": "complete", "data": f"{count} deleted"})
    return jsonify({"status": "failed", "data": "not found"})


# ── DELETE RANGE: remove all documents between two timestamps ──
# DELETE /api/weather/delete/<start>/<end>
@app.route('/api/weather/delete/<start>/<end>', methods=['DELETE'])
def delete_weather_range(start, end):
    count = mongo.delete_weather_range(start, end)
    return jsonify({"status": "complete", "data": f"{count} deleted"})


# ── COUNT: number of documents in a time range ──
# GET /api/weather/count/<start>/<end>
@app.route('/api/weather/count/<start>/<end>', methods=['GET'])
def count_weather(start, end):
    count = mongo.count_weather_data(start, end)
    return jsonify({"status": "found", "data": count})