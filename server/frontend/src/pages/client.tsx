import {
  Alert,
  Collapsible,
  Container,
  // createListCollection,
  Field,
  For,
  HStack,
  IconButton,
  Input,
  // Select,
  Table,
  Tabs,
} from "@chakra-ui/react";
import {IoFilter, IoReload} from "react-icons/io5";
import {useEffect, useState} from "react";
import {listConnectedClients, updateConnectedClients} from "../services/client.ts";

function Client() {
  const [flagClientsError, setFlagClientsError] = useState<boolean>(false);
  const [clients, setClients] = useState<{
    "key": number,
    "ip_addr": string,
    "port": number,
    "type": number,
    "stream_port": number,
    "user_addr": string,
    "user_port": number,
    "main_addr": string,
    "main_port": number
  }[]>([]);
  const [filterClient, setFilterClient] = useState<RegExp>(/./);
  const [filterStreamPort, setFilterStreamPort] = useState<RegExp>(/./);
  // const clientTypes = createListCollection({
  //   items: [
  //     {label: "connection", value: 0},
  //     {label: "proxy", value: 4}
  //   ]
  // });
  const [filterUser, setFilterUser] = useState<RegExp>(/./);
  const [filterMain, setFilterMain] = useState<RegExp>(/./);

  useEffect(() => {
    (async () => {
      const res = await listConnectedClients();
      if (res !== null) {
        setFlagClientsError(false);
        setClients(res);
      } else {
        setFlagClientsError(true);
      }
    })();
  }, []);

  return (
    <Container w="100%" h="100%" p="1rem">
      <Tabs.Root variant="subtle">
        <Tabs.List>
          <Tabs.Trigger value="Connected">Connected</Tabs.Trigger>
          <Tabs.Trigger value="Usage">All Time Usage</Tabs.Trigger>
        </Tabs.List>

        <Alert.Root status="error" m={3} hidden={!flagClientsError}>
          <Alert.Indicator/>
          <Alert.Content>
            <Alert.Title>Connection Lost</Alert.Title>
            <Alert.Description>
              Unable to connnect to api server
            </Alert.Description>
          </Alert.Content>
        </Alert.Root>

        <Tabs.Content value="Connected">
          <Collapsible.Root>
            <HStack m="2px">
              <IconButton onClick={(async () => {
                if (!await updateConnectedClients()) {
                  setFlagClientsError(true);
                }
                const res = await listConnectedClients();
                if (res !== null) {
                  setFlagClientsError(false);
                  setClients(res);
                } else {
                  setFlagClientsError(true);
                }
              })} variant="ghost" size="sm">
                <IoReload/>
              </IconButton>

              <Collapsible.Trigger>
                <IconButton variant="ghost" size="sm">
                  <IoFilter/>
                </IconButton>
              </Collapsible.Trigger>
            </HStack>

            <Collapsible.Content>
              <HStack p="3px">
                <Field.Root orientation="horizontal">
                  <Input placeholder="client" onChange={
                    (event) => setFilterClient((event.target.value != "") ? RegExp(event.target.value) : /./)
                  }/>
                  {/*<Select.Root multiple collection={clientTypes}>*/}
                  {/*  <Select.Control>*/}
                  {/*    <Select.Trigger>*/}
                  {/*      <Select.ValueText placeholder="client type"/>*/}
                  {/*    </Select.Trigger>*/}
                  {/*    <Select.IndicatorGroup>*/}
                  {/*      <Select.Indicator/>*/}
                  {/*    </Select.IndicatorGroup>*/}
                  {/*  </Select.Control>*/}
                  {/*  <Select.Positioner>*/}
                  {/*    <Select.Content>*/}
                  {/*      {clientTypes.items.map((type) => (*/}
                  {/*        <Select.Item item={type} key={type.value}>*/}
                  {/*          {type.label}*/}
                  {/*          <Select.ItemIndicator/>*/}
                  {/*        </Select.Item>*/}
                  {/*      ))}*/}
                  {/*    </Select.Content>*/}
                  {/*  </Select.Positioner>*/}
                  {/*</Select.Root>*/}
                  <Input placeholder="stream port" onChange={
                    ((event) => {
                      setFilterStreamPort(() => {
                        if (event.target.value == "") {
                          return /./;
                        }
                        if (/^\d*$/.test(event.target.value) && Number(event.target.value) > 0 && Number(event.target.value) <= 65535) {
                          return RegExp("^" + event.target.value + "\\d*$");
                        }
                        return /./;
                        //  TODO: invalid port warning
                      });
                      console.log(filterStreamPort);
                      console.log(filterStreamPort.test("51000"));
                    })
                  }/>
                  <Input placeholder="user" onChange={
                    (event) => setFilterUser((event.target.value != "") ? RegExp(event.target.value) : /./)
                  }/>
                  <Input placeholder="client main" onChange={
                    (event) => setFilterMain((event.target.value != "") ? RegExp(event.target.value) : /./)
                  }/>
                </Field.Root>
              </HStack>
            </Collapsible.Content>
          </Collapsible.Root>

          <Table.Root>
            <Table.Header>
              <Table.Row>
                <Table.ColumnHeader>Client IP</Table.ColumnHeader>
                <Table.ColumnHeader>Client Type</Table.ColumnHeader>
                <Table.ColumnHeader>Stream Port</Table.ColumnHeader>
                <Table.ColumnHeader>User IP</Table.ColumnHeader>
                <Table.ColumnHeader>Client Main IP</Table.ColumnHeader>
              </Table.Row>
            </Table.Header>

            <Table.Body>
              <For each={clients.filter((client) =>
                filterClient.test(client.ip_addr + ':' + client.port)
                && filterStreamPort.test(client.stream_port.toString())
                && filterUser.test(client.user_addr + ':' + client.user_port)
                && filterMain.test(client.main_addr + ':' + client.main_port)
              )}>
                {item => (
                  <Table.Row key={item.key}>
                    <Table.Cell>{item.ip_addr + ':' + item.port}</Table.Cell>
                    <Table.Cell>{(() => {
                      switch (item.type) {
                        case 0:
                          return "connection";
                        case 4:
                          return "proxy";
                        default:
                          return "-";
                      }
                    })()
                    }</Table.Cell>
                    <Table.Cell>{(item.type == 0) ? item.stream_port : "-"}</Table.Cell>
                    <Table.Cell>{(item.type == 4) ? (item.user_addr + ':' + item.user_port) : "-"}</Table.Cell>
                    <Table.Cell>{(item.type == 4) ? (item.main_addr + ':' + item.main_port) : "-"}</Table.Cell>
                  </Table.Row>
                )
                }
              </For>
            </Table.Body>
          </Table.Root>
        </Tabs.Content>
      </Tabs.Root>
    </Container>
  );
}

export default Client;
