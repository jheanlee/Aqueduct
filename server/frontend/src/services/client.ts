import {cronFetcher, fetcher} from "../core/fetcher";
import {isAxiosError} from "axios";

export const getStatus = async () => {
  try {
    const res = await cronFetcher
      .get<{
        api_service_up: boolean;
        connected_clients: number;
        tunnel_service_up: boolean;
        uptime: number;
      }>("/api/status");
    return res.data;
  } catch (error) {
    if (isAxiosError(error)) {
      return error.status || 500;
    }
    return 500;
  }
};

export const listConnectedClients = async () => {
  try {
    const res = await cronFetcher
      .get<
        {
          key: number;
          ip_addr: string;
          port: number;
          type: number;
          stream_port: number;
          user_addr: string;
          user_port: number;
          main_addr: string;
          main_port: number;
        }[]
      >("/api/clients/connected");
    return res.data;
  } catch (error) {
    if (isAxiosError(error)) {
      return error.status || 500;
    }
    return 500;
  }
};

export const updateConnectedClients = async () => {
  try {
    await fetcher.post("/api/clients/update");
    return 200;
  } catch (error) {
    if (isAxiosError(error)) {
      return error.status || 500;
    }
    return 500;
  }
};

export const listClientsDb = async () => {
  try {
    const res = await fetcher
      .get<
        {
          ip: string;
          sent: number;
          received: number;
        }[]
      >("/api/clients/db");
    return res.data;
  } catch (error) {
    if (isAxiosError(error)) {
      return error.status || 500;
    }
    return 500;
  }
};
