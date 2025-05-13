import {
  Container,
  Grid,
  GridItem,
  Text,
  Alert,
  Box,
  Status,
  Stack,
  Heading,
} from "@chakra-ui/react";
import { useEffect, useState } from "react";
import { getStatus } from "../services/client.ts";

function Dashboard() {
  const [flagStatusError, setFlagStatusError] = useState<boolean>(false);
  const [uptime, setUptime] = useState<number>(0);
  const [tunnelServiceUp, setTunnelService] = useState<boolean>(false);
  const [apiServiceUp, setApiService] = useState<boolean>(false);
  const [connectedClients, setConnectedClients] = useState<number>(0);

  useEffect(() => {
    const updateStatus = async () => {
      const res = await getStatus();
      if (typeof res === "number") {
        setFlagStatusError(true);
      } else {
        setFlagStatusError(false);
        setUptime(res["uptime"]);
        setTunnelService(res["tunnel_service_up"]);
        setApiService(res["api_service_up"]);
        setConnectedClients(res["connected_clients"]);
      }
    }

    (async () => await updateStatus()) ();

    const interval = setInterval(async () => await updateStatus(), 5000);

    return () => clearInterval(interval);
  }, []);

  return (
    <Container w="100%" h="100%" p="1rem">
      <Grid w="100%" h="100%" gap={5} p={10}>
        <GridItem rowSpan={1} colSpan={1}>
          {flagStatusError && (
            <Alert.Root status="error">
              <Alert.Indicator />
              <Alert.Content>
                <Alert.Title>Connection Lost</Alert.Title>
                <Alert.Description>
                  Unable to connect to api server
                </Alert.Description>
              </Alert.Content>
            </Alert.Root>
          )}

          {!flagStatusError && (
            <Box p={10} bg="bg.muted">
              <Stack>
                <Heading>Overview</Heading>
                <FormatUptime uptime={uptime} />
                <Status.Root colorPalette={tunnelServiceUp ? "green" : "red"}>
                  <Status.Indicator />
                  {"Tunnel Service: " +
                    (tunnelServiceUp ? "Online" : "Offline")}
                </Status.Root>
                <Status.Root colorPalette={apiServiceUp ? "green" : "red"}>
                  <Status.Indicator />
                  {"API Service: " + (apiServiceUp ? "Online" : "Offline")}
                </Status.Root>
                <Text>
                  {"Connected Clients: " + connectedClients.toString()}
                </Text>
              </Stack>
            </Box>
          )}
        </GridItem>
      </Grid>
    </Container>
  );
}

type UptimeProp = {
  uptime: number;
};

function FormatUptime({ uptime }: UptimeProp) {
  const seconds = uptime % 60;
  uptime /= 60;
  const minutes = Math.floor(uptime % 60);
  uptime /= 60;
  const hours = Math.floor(uptime % 60);
  uptime /= 24;
  const days = Math.floor(uptime);

  return (
    <Text>
      {`Uptime: ${days.toString()}d:${hours.toString().padStart(2, "0")}h:${minutes.toString().padStart(2, "0")}m:${seconds.toString().padStart(2, "0")}s`}
    </Text>
  );
}

export default Dashboard;
