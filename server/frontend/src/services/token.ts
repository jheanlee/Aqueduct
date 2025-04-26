import { fetcher } from "../core/fetcher";

export const listTokens = async () => {
  return await fetcher
    .get<
      {
        name: string;
        token: string;
        notes: string | null;
        expiry: number | null;
      }[]
    >("/api/tokens/list")
    .then((res) => {
      return res.data;
    })
    .catch(() => {
      return null;
    });
};

export const checkToken = async (name: string) => {
  return await fetcher
    .get<{
      available: boolean;
    }>("/api/tokens/check", {
      params: {
        name: name,
      },
    })
    .then((res) => {
      return res.data.available;
    })
    .catch(() => {
      return null;
    });
};

export const modifyTokens = async (data: {
  name: string;
  token_update: boolean;
  notes: string | null;
  expiry_days: number | null;
}) => {
  return await fetcher
    .post<{
      token: string;
    }>("/api/tokens/modify", data)
    .then((res) => {
      return res.data;
    })
    .catch(() => {
      return null;
    });
};

export const deleteToken = async (name: string) => {
  return await fetcher
    .post<{
      rows_affected: number;
    }>("/api/tokens/delete", { name: name })
    .then((res) => {
      return res.data;
    })
    .catch(() => {
      return null;
    });
};
