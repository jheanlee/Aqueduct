import { fetcher } from "../core/fetcher.ts";
import { isAxiosError } from "axios";

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
