import { fetcher } from "../core/fetcher.ts";
import { isAxiosError } from "axios";
import {toaster} from "../components/ui/toaster.tsx";

export const login = async (data: { username: string; password: string }) => {
  try {
    const res = await fetcher.post<{
      token: string;
    }>("api/users/login", data);
    fetcher.defaults.headers['Authorization'] = res.data.token;
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
      if (error.status === 401) {
        toaster.create({
          description: "Session expired",
          type: "error",
        });
      }
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
      if (error.status === 401) {
        toaster.create({
          description: "Session expired",
          type: "error",
        });
      }
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
      if (error.status === 401) {
        toaster.create({
          description: "Session expired",
          type: "error",
        });
      }
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
      if (error.status === 401) {
        toaster.create({
          description: "Session expired",
          type: "error",
        });
      }
      return error.status || 500;
    }
    return 500;
  }
}
