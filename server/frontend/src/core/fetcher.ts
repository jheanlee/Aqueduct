import axios from "axios";
import { refresh_token } from "../services/auth.ts";
import { Buffer } from "buffer";


// public fetcher (login & refresh token)
export const publicFetcher = axios.create();


// cron job
export const cronFetcher = axios.create();

cronFetcher.interceptors.request.use(async (config) => {
  const access_token = localStorage.getItem("aqueduct.access_token");
  if (access_token === null) {
    window.location.href="/#/login";
    return config;
  }

  cronFetcher.defaults.headers["Authorization"] = access_token;
  if (JSON.parse(Buffer.from(access_token.split('.')[1], "base64").toString("ascii")).exp < Date.now() / 1000) {
    window.location.href="/#/login";
    return config;
  }
  return config;
})


// private fetcher (user conducted calls)
export const fetcher = axios.create();

fetcher.interceptors.request.use(async (config)=> {
  const access_token = localStorage.getItem("aqueduct.access_token");
  if (access_token === null) {
    window.location.href="/#/login";
    return config;
  }

  fetcher.defaults.headers["Authorization"] = access_token;
  if (JSON.parse(Buffer.from(access_token.split('.')[1], "base64").toString("ascii")).exp < Date.now() / 1000 + 300) {
    const res = await refresh_token();
    if (res !== 200) {
      window.location.href="/#/login";
      return config;
    }
  }
  return config
})




