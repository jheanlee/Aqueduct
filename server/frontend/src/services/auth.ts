import {cronFetcher, fetcher, publicFetcher} from "../core/fetcher.ts";
import { isAxiosError } from "axios";

export const refresh_token = async () => {
  try {
    const res = await publicFetcher.post<{
      access_token: string
    }>("api/refresh-token", {
      refresh_token: localStorage.getItem("aqueduct.refresh_token"),
      access_token: localStorage.getItem("aqueduct.access_token")
    })
    localStorage.setItem("aqueduct.access_token", res.data.access_token);
    fetcher.defaults.headers["Authorization"] = res.data.access_token;
    cronFetcher.defaults.headers["Authorization"] = res.data.access_token;
    return 200;
  } catch (error) {
    if (isAxiosError(error)) {
      return error.status || 500;
    }
    return 500;
  }
}

export const login = async (data: { username: string; password: string }) => {
  try {
    const res = await publicFetcher.post<{
      refresh_token: string,
      access_token: string
    }>("api/users/login", data);
    localStorage.setItem("aqueduct.refresh_token", res.data.refresh_token);
    localStorage.setItem("aqueduct.access_token", res.data.access_token);
    fetcher.defaults.headers["Authorization"] = res.data.access_token;
    cronFetcher.defaults.headers["Authorization"] = res.data.access_token;
    return 200;
  } catch (error) {
    if (isAxiosError(error)) {
      return error.status || 500;
    } else {
      return 500;
    }
  }
};

export const listUsers = async () => {
  try {
    const res = await fetcher.get<
      {
        "username": string
      }[]
    >("api/users/list");
    return res.data;
  } catch (error) {
    if (isAxiosError(error)) {
      return error.status || 500;
    }
    return 500;
  }
}

export const checkUser = async (params: {
  username: string
}) => {
  try {
    const res = await fetcher.get<{available: boolean}>("api/users/check", {
      params: params
    });
    return res.data;
  } catch (error) {
    if (isAxiosError(error)) {
      return error.status || 500;
    }
    return 500;
  }
}

export const modifyUser = async (data: {
  username: string,
  password: string
}) => {
  try {
    await fetcher.post("api/users/modify", data);
    return 200;
  } catch (error) {
    if (isAxiosError(error)) {
      return error.status || 500;
    }
    return 500;
  }
}

export const deleteUser = async (data: {
  username: string
}) => {
  try {
    const res = await fetcher.post<{rows_affected: number}> ("api/users/delete", data);
    return res.data;
  } catch (error) {
    if (isAxiosError(error)) {
      return error.status || 500;
    }
    return 500;
  }
}
