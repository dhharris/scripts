# -*- coding: utf-8 -*-

# Sample Python code for youtube.playlistItems.list
# See instructions for running these code samples locally:
# https://developers.google.com/explorer-help/guides/code_samples#python

import argparse
import os
import subprocess
import sys
from typing import List

scopes = ["https://www.googleapis.com/auth/youtube.readonly"]
packages = ["grequests", "google-api-python-client", "google-auth-oauthlib google-auth-httplib2"]

def update_api_libs():
    FNULL = open(os.devnull, "w")
    args = "-m pip install {} --upgrade"
    for pkg in packages:
        subprocess.check_call(
                [sys.executable] + args.format(pkg).split(' '),
                stdout=FNULL)
    FNULL.close()


def get_playlist_video_ids(playlist_id: str) -> List[str]:
    import googleapiclient.discovery
    import googleapiclient.errors

    print(f"Getting videos in playlist {playlist_id}")

    api_service_name = "youtube"
    api_version = "v3"

    # Get credentials and create an API client
    with open("youtube_api_key.json", "r") as f:
        api_key = f.readline().rstrip("\n")
    youtube = googleapiclient.discovery.build(
        api_service_name, api_version, developerKey=api_key)

    request = youtube.playlistItems().list(
        part="contentDetails",
        maxResults=50,
        playlistId=playlist_id,
    )
    response = request.execute()
    if "items" in response:
        return [entry["contentDetails"]["videoId"] for entry in response["items"]]
    return []

def download_videos(video_ids: List[str]):
    import grequests

    print(f"Downloading {len(video_ids)} videos")
    youtube_video_url = "https://www.youtube.com/watch?v={}"
    youtube_dl_url = "http://hughscloud.com:9835"
    with open("token.txt", "r") as f:
        token = f.readline().rstrip("\n")
    requests = (grequests.get(
        youtube_dl_url,
        params={"token": token, "url": youtube_video_url.format(i)})
        for i in video_ids
    )
    results = grequests.map(requests, size=5)

    good_status = [i for i in results if i.status_code == 200]
    succeeded = [i for i in results if i.text] # text should be non-empty
    if len(succeeded) != len(video_ids):
        print("Some requests did not succeed")
    else:
        print("Success!")


def main():
    parser = argparse.ArgumentParser(description="""
Downloads videos from the given YouTube playlist
""")
    parser.add_argument("playlist_id", metavar="id", type=str, nargs=1,
        help="IDs of a YouTube playlist")
    args = parser.parse_args()
    update_api_libs()
    videos = get_playlist_video_ids(args.playlist_id[0])
    download_videos(videos)

if __name__ == "__main__":
    main()
