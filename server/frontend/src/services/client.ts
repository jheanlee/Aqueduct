import {fetcher} from "../core/fetcher";

export const getStatus = async () => {
  return await fetcher.get<{
    "api_service_up": boolean,
    "connected_clients": number,
    "tunnel_service_up": boolean,
    "uptime": number
  }>("/api/status")
    .then((res) => {
      return res.data;
    })
    .catch(() => {
      return null;
    });
}

export const listConnectedClients = async () => {
  return await fetcher.get<{
    "key": number,
    "ip_addr": string,
    "port": number,
    "type": number,
    "stream_port": number,
    "user_addr": string,
    "user_port": number,
    "main_addr": string,
    "main_port": number
  }[]>("/api/clients/connected")
    .then((res) => {
      return res.data;
    })
    .catch(() => {
      return null;
    });
}

export const updateConnectedClients = async () => {
  return await fetcher.post("/api/clients/update").catch(() => {
    return null
  });
}

export const listClientsDb = async () => {
  return await fetcher.get<{
    "ip": string,
    "sent": number,
    "received": number
  }[]>("/api/clients/db")
    .then((res) => {
      return res.data;
    })
    .catch(() => {
      return null;
    });
}