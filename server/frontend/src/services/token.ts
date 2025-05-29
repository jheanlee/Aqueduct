import { fetcher } from "../core/fetcher";
import { isAxiosError } from "axios";

export const listTokens = async () => {
  try {
    const res = await fetcher
      .get<
        {
          name: string;
          notes: string | null;
          expiry: number | null;
        }[]
      >("/api/tokens/list");
    return res.data;
  } catch (error) {
    if (isAxiosError(error)) {
      return error.status || 500;
    }
    return 500;
  }
};

export const checkToken = async (name: string) => {
  try {
    const res = await fetcher
      .get<{
        available: boolean;
      }>("/api/tokens/check", {
        params: {
          name: name,
        },
      });
    return res.data;
  } catch (error) {
    if (isAxiosError(error)) {
      return error.status || 500;
    }
    return 500;
  }
};

export const modifyToken = async (data: {
  name: string;
  token_update: boolean;
  notes: string | null;
  expiry_days: number | null;
}) => {
  try {
    const res = await fetcher
      .post<{
        token: string;
      }>("/api/tokens/modify", data);
    return res.data;
  } catch (error) {
    if (isAxiosError(error)) {
      return error.status || 500;
    }
    return 500;
  }

};

export const deleteToken = async (name: string) => {
  try {
    const res = await fetcher
      .post<{
        rows_affected: number;
      }>("/api/tokens/delete", { name: name });
    return res.data;
  } catch (error) {
    if (isAxiosError(error)) {
      return error.status || 500;
    }
    return 500;
  }
};
