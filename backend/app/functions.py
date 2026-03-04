 #!/usr/bin/python3


#################################################################################################################################################
#                                                    CLASSES CONTAINING ALL THE APP FUNCTIONS                                                                                                    #
#################################################################################################################################################


class DB:

    def __init__(self,Config):

        from math import floor
        from os import getcwd
        from os.path import join
        from json import loads, dumps, dump
        from datetime import timedelta, datetime, timezone 
        from pymongo import MongoClient , errors, ReturnDocument
        from urllib import parse
        from urllib.request import  urlopen 
        from bson.objectid import ObjectId  
       
      
        self.Config                         = Config
        self.getcwd                         = getcwd
        self.join                           = join 
        self.floor                      	= floor 
        self.loads                      	= loads
        self.dumps                      	= dumps
        self.dump                       	= dump  
        self.datetime                       = datetime
        self.ObjectId                       = ObjectId 
        self.server			                = Config.DB_SERVER
        self.port			                = Config.DB_PORT
        self.username                   	= parse.quote_plus(Config.DB_USERNAME)
        self.password                   	= parse.quote_plus(Config.DB_PASSWORD)
        self.remoteMongo                	= MongoClient
        self.ReturnDocument                 = ReturnDocument
        self.PyMongoError               	= errors.PyMongoError
        self.BulkWriteError             	= errors.BulkWriteError  
        self.tls                            = False # MUST SET TO TRUE IN PRODUCTION


    def __del__(self):
            # Delete class instance to free resources
            pass
 


####################
    # LAB 4 FUNCTIONS  #
    ####################
    
    # 1. CREATE FUNCTION TO INSERT DATA IN TO THE RADAR COLLECTION
    def insert_radar_data(self, data):
        try:
            remotely = self.remoteMongo(f"mongodb://{self.username}:{self.password}@{self.server}:{self.port}")
            db = remotely.ELET2415
            return db.radar.insert_one(data)
        except self.PyMongoError as e:
            print(f"Insert Error: {e}")
            return None

    # 2. CREATE FUNCTION TO RETRIEVE ALL DOCUMENTS FROM RADAR COLLECT BETWEEN SPECIFIED DATE RANGE
    def get_radar_range(self, start, end):
        try:
            remotely = self.remoteMongo(f"mongodb://{self.username}:{self.password}@{self.server}:{self.port}")
            db = remotely.ELET2415
            return list(db.radar.find({"timestamp": {"$gte": int(start), "$lte": int(end)}}, {"_id": 0}))
        except self.PyMongoError as e:
            print(f"Range Error: {e}")
            return []

    # 3. CREATE A FUNCTION TO COMPUTE THE ARITHMETIC AVERAGE ON THE 'reserve' FIELD
    def get_reserve_average(self, start, end):
        try:
            remotely = self.remoteMongo(f"mongodb://{self.username}:{self.password}@{self.server}:{self.port}")
            db = remotely.ELET2415
            pipeline = [
                {"$match": {"timestamp": {"$gte": int(start), "$lte": int(end)}}},
                {"$group": {"_id": None, "average": {"$avg": "$reserve"}}},
                {"$project": {"_id": 0, "average": 1}}
            ]
            return list(db.radar.aggregate(pipeline))
        except self.PyMongoError as e:
            print(f"Average Error: {e}")
            return []
    
    # 4. CREATE A FUNCTION THAT INSERT/UPDATE A SINGLE DOCUMENT IN THE 'code' COLLECTION
    def set_passcode(self, passcode):
        try:
            remotely = self.remoteMongo(f"mongodb://{self.username}:{self.password}@{self.server}:{self.port}")
            db = remotely.ELET2415
            return db.code.find_one_and_update(
                {"type": "passcode"},
                {"$set": {"code": passcode}},
                upsert=True,
                return_document=self.ReturnDocument.AFTER
            )
        except self.PyMongoError as e:
            print(f"Set Passcode Error: {e}")
            return None
    
    # 5. CREATE A FUNCTION THAT RETURNS A COUNT OF DOCUMENTS MATCHING THE PASSCODE
    def check_passcode(self, passcode):
        try:
            remotely = self.remoteMongo(f"mongodb://{self.username}:{self.password}@{self.server}:{self.port}")
            db = remotely.ELET2415
            return db.code.count_documents({"type": "passcode", "code": passcode})
        except self.PyMongoError as e:
            print(f"Check Passcode Error: {e}")
            return 0



def main():
    from config import Config
    from time import time, ctime, sleep
    from math import floor
    from datetime import datetime, timedelta
    one = DB(Config)
 
 
    start = time() 
    end = time()
    print(f"completed in: {end - start} seconds")
    
if __name__ == '__main__':
    main()


    
